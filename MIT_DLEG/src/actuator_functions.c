
/*****************************************************************************
	[Lead developers] Matt Carney, mcarney at mit dot edu
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors] Matthew Carney, mcarney at mit dot edu, Tony Shu, tonyshu at mit dot edu
*****************************************************************************
	[This file] MIT DARPA Leg Actuator Specific functions
****************************************************************************/


//****************************************************************************
// Include(s)
//****************************************************************************

#include "actuator_functions.h"


//****************************************************************************
// Variable(s)
//****************************************************************************

//Variables which aren't static may be updated by Plan in the future

//extern uint8_t mitDlegInfo[2];// = {PORT_RS485_2, PORT_RS485_2};

//SAFETY FLAGS - in addition to enum, so can be cleared but don't lose other flags that may exist.
static int8_t isSafetyFlag = 0;
static int8_t isAngleLimit = 0;
static int8_t isTorqueLimit = 0;
static int8_t isTempLimit = 0;
static int8_t startedOverLimit = 1;

int8_t isEnabledUpdateSensors = 0;
int8_t fsm1State = STATE_POWER_ON;
float currentScalar = CURRENT_SCALAR_INIT;

int8_t zeroIt = 0;	//Used to allow re-zeroing of the load cell, ie for testing purposes
float voltageGain = 1.0;	//tested
float velGain = 1.1;	// tested
float indGain = 1.73;	// tested


//torque gain values
// ARE THESE WORKING? CHECK THAT THEY'RE NOT BEING OVERWRITTEN BY USERWRITES!!!
float torqueKp = TORQ_KP_INIT;
float torqueKi = TORQ_KI_INIT;
float torqueKd = TORQ_KD_INIT;
float errorKi = 0.0;


//motor param terms
float motJ = MOT_J;
float motB = MOT_B;



//const vars taken from defines (done to speed up computation time)
static const float angleUnit    = ANG_UNIT;
static const float jointZeroAbs = JOINT_ZERO_ABS;
static const float jointZero	= JOINT_ZERO;

static const float forcePerTick  = FORCE_PER_TICK;

static const float jointMinSoft = JOINT_MIN_SOFT;
static const float jointMaxSoft = JOINT_MAX_SOFT;
static const float jointMinSoftDeg = JOINT_MIN_SOFT * DEG_PER_RAD;
static const float jointMaxSoftDeg = JOINT_MAX_SOFT * DEG_PER_RAD;

struct diffarr_s jntAngClks;		//maybe used for velocity and accel calcs.


//****************************************************************************
// Method(s)
//****************************************************************************

/**
 * The ADC reads the motor Temp sensor - MCP9700 T0-92.
 * Return: motTemp(int16_t) - The temperature of the motor in CELCIUS
 */
static int16_t getMotorTempSensor(void)
{
	static int16_t motTemp = 0;
	motTemp = (rigid1.mn.analog[0] * (3.3/4096) - 500) / 10; 	//celsius
	rigid1.mn.mot_temp = motTemp;

	return motTemp;
}

/*
 * Output joint angle, vel, accel in ANG_UNIT, measured from joint zero,
 * Param: actx(struct act_s) - Actuator structure to track sensor values
 * updated Actuator structure:
 * 		actx->jointAngle = new angle
 * 		actx->jointVel = new velocity
 * 		actx->jointAcc = new acceleration
 * 		//saves previous values for next use
 * 		actx->lastJointAngle = angle
 *		TODO:Make lastJointVel part of act_s
 */
static void getJointAngleKinematic(struct act_s *actx)
{
	static float lastJointVel = 0;
	static float jointAngleRad = 0;

	//ANGLEReturn: tauRestoring(float) -
	jointAngleRad = JOINT_ANGLE_DIR * ( jointZero + JOINT_ENC_DIR * ( (float) (*(rigid1.ex.joint_ang)) ) )  * RAD_PER_CNT; // (angleUnit)/JOINT_CPR;

	// filter joint angle? TODO: maybe remove this. or use windowAveraging
	actx->jointAngle = 0.8*actx->jointAngle + 0.2*jointAngleRad;

	//VELOCITY
	actx->jointVel = 0.8*actx->jointVel + 0.2*( (actx->jointAngle - actx->lastJointAngle) * SECONDS);

	//ACCEL  -- todo: check to see if this works
	actx->jointAcc = 0.8 * actx->jointAcc + 0.2*( (actx->jointVel - lastJointVel ) * SECONDS);

	// SAFETY CHECKS
	//if we start over soft limits after findPoles(), only turn on motor after getting within limits
	//TODO: I think we can remove this, verify we are handling this with safety functions
	if (startedOverLimit && jointAngleRad < jointMaxSoft && jointAngleRad > jointMinSoft) {
		startedOverLimit = 0;
	}

	// Save values for next time through.
	actx->lastJointAngle = actx->jointAngle;
	lastJointVel = actx->jointVel;

}

/*
 *  Filter using moving average.  This one works well for small window sizes
 *  larger windowsizes gets better accuracy with windowAverageingLarge
 *  uses WINDOW_SIZE
 *  Param: currentVal(float) - current value given
 *  Return: average(float) - e rolling average of all previous and current values
 */
//UNDERGRAD TODO: figure out whythis isn't working
static float windowAveraging(float currentVal) {

	static int16_t index = -1;
	static float window[WINDOW_SIZE];
	static float average = 0;

	index = (index + 1) % WINDOW_SIZE;
	average = average - window[index]/WINDOW_SIZE;
	window[index] = currentVal;
	average = average + window[index]/WINDOW_SIZE;

	return average;
}

/**
 * Output axial force on screw
 * Return: axialForce(float) -  Force on the screw in NEWTONS
 */
static float getAxialForce(struct act_s *actx, int8_t tare)
{
	static int8_t tareState = -1;
	static uint32_t timer = 0;
	float strainReading = 0;
	static float tareOffset = 0;
	float axialForce = 0;
	float numSamples = 1000.;
	float timerDelay = 100.;

	strainReading = (float) rigid1.ex.strain;

	if (tare)
	{	// User input has requested re-zeroing the load cell, ie locked output testing.
		tareState = -1;
		timer = 0;
		tareOffset = 0;
		tare=0;

	}

	switch(tareState)
	{
		case -1:
			//Tare the balance using average of numSamples readings
			timer++;

			if(timer >= timerDelay && timer < numSamples + timerDelay) {
				tareOffset += (strainReading)/numSamples;
			} else if (timer >= numSamples + timerDelay) {
				tareState = 0;
			}

			break;

		case 0:

			axialForce =  FORCE_DIR * (strainReading - tareOffset) * forcePerTick;

			// Filter the signal

//			axialForce = runSoftFirFilt(axialForce);
			axialForce = runButterworthFiltMeasurements(axialForce);

			break;


		default:
			//problem occurred
			break;
	}

	return axialForce;
}




/**
 * Linear Actuator Actual Moment Arm
 * Param: theta(float) - the angle of the joint in RADIANS
 * Return: projLength(float) - moment arm projected length in METERS
 */
static float getLinkageMomentArm(struct act_s *actx, float theta, int8_t tareState)
{
	static float A=0, c0=0, c = 0, c2 = 0, projLength = 0, CAng = 0;
	static float deltaLengthMotorMeas = 0, deltaMotorCalc=0;
	static float screwTravelPerMotorCnt = (float) (L_SCREW/MOTOR_COUNTS_PER_REVOLUTION);

	CAng = M_PI - theta - (MA_TF); 	// angle
    c2 = MA_A2B2 - MA_TWOAB* cosf(CAng);
    c = sqrtf(c2);  // length of actuator from pivot to output

    A = acosf(( MA_A2MINUSB2 - c2 ) / (-2*MA_B*c) );

    projLength = (MA_B * sinf(A))/1000.;

    if (tareState == 1)
    {
    	c0 = c;	// save initial length of actuator	//NOTE: THIS SEEMS TO OUTPUT A FUNNY VALUE
    	actx->motorPosNeutral = *rigid1.ex.enc_ang;
    	tareState = 0;
    }

    // Keep track of spring deflection, expected and actual
//    rigid1.mn.genVar[8] = (int16_t) (c0 * 10);
    actx->screwLengthDelta = (c-c0); // This is deflection of spring [m]
    actx->motorPosDelta = (actx->motorPosRaw - actx->motorPosNeutral);


    deltaLengthMotorMeas = screwTravelPerMotorCnt * ( (float) actx->motorPosDelta ); // correct
    actx->linkageLengthNonLinearity = deltaLengthMotorMeas - actx->screwLengthDelta;	// Difference for now, todo: use to calc force. NOT WORKING YET

    return projLength;

}


/*
 *  Determine torque at joint due to moment arm and axial force
 *  Param:	actx(struct act_s) -  Actuator structure to track sensor values
 *  Return: torque(float) - joint torque in NEWTON-METERS
 *  //todo: more accurate is to track angle of axial force. maybe at getLinkageMomentArm
 */
static float getJointTorque(struct act_s *actx)
{
	float torque = 0;

	torque = actx->linkageMomentArm * actx->axialForce;

	torque = torque * TORQ_CALIB_M + TORQ_CALIB_B;		//apply calibration to torque measurement

	if(torque >= ABS_TORQUE_LIMIT_INIT || torque <= -ABS_TORQUE_LIMIT_INIT) {
		isSafetyFlag = SAFETY_TORQUE;
		isTorqueLimit = 1;
	} else {
		isTorqueLimit = 0;
	}

	return torque;
}

/*
 * 	Determine rate of change of motor current
 * 	Param: Act_s
 * 	return: di/dt
 */
static float getMotorCurrentDt(struct act_s *actx)
{
	static float lastCurrent = 0.0;

	float currentDt = (actx->motCurr - lastCurrent);	// units [A/sec]

	currentDt = 0.8*lastCurrent + 0.2 * currentDt;

	return currentDt;
}

/*
 *  Determine torque rate at joint using window averaging
 *  Param:	actx(struct act_s) - Actuator structure to track sensor values
 *  Return: average(float) - joint torque rate in NEWTON-METERS PER SECOND
 */
static float windowJointTorqueRate(struct act_s *actx) {
	#define TR_WINDOW_SIZE 3

	static int8_t index = -1;
	static float window[TR_WINDOW_SIZE];
	static float average = 0;
	static float previousTorque = 0;
	float currentRate = 0;

	index = (index + 1) % TR_WINDOW_SIZE;
	currentRate = (actx->jointTorque - previousTorque)*SECONDS;
	average -= window[index]/TR_WINDOW_SIZE;
	window[index] = currentRate;
	average += window[index]/TR_WINDOW_SIZE;

	previousTorque = actx->jointTorque;

	return average;

}




/*
 * Update the joint torque rate in the Actuator structure
 * Param: actx(struct act_s) - Actuator structure to track sensor values
 * updated Actuator structure:
 * 		actx->lastJointTorque = current joint torque rate
 * 		actx->jointTorqueRate = new joint torque rate
 */
static void updateJointTorqueRate(struct act_s *actx){

	//TODO: consider switching to windowAveraging

	float diff = actx->jointTorque - actx->lastJointTorque;
    actx->jointTorqueRate = 0.8 * actx->jointTorqueRate + 0.2 *( SECONDS * (diff) );
    actx->lastJointTorque = actx->jointTorque;
}

/*
 * Anti-windup clamp for integral term.
 * Check if output is saturating, and if error in the same direction
 * Clamp the integral term if that's the case.
 *  Param:  tauErr(float) -
 *  Param:  tauCTotal(float) -
 *  Param:  tauCOutput(float) -
 *  Return: saturation(bool) - whether the output is saturating or not
 */
bool integralAntiWindup(float tauErr, float tauCTotal, float tauCOutput) {
	//Anti-windup clamp for Integral Term of PID
	int8_t inSatLimit = 0;
	if (tauCTotal == tauCOutput){
		inSatLimit = 0;
	} else {
		inSatLimit = 1;
	}

	//check if the error is increasing  in teh direction of saturation
	int8_t errSign = 0;
	if (tauErr > 0 && tauCTotal > 0) {
		errSign = 1;
	} else if (tauErr < 0 && tauCTotal < 0) {
		errSign = 1;//-1;
	} else {
		errSign = 0;
	}

	// if saturated and error is going in that direction use antiwindup.
//	if (inSatLimit && errSign) {
//		return 1;
//	} else {
//		return 0;
//	}
	return inSatLimit;
}

/*
 * Check for angle limits and apply virtual unidirectional spring/dampers
 *  Param:	actx(struct act_s) - Actuator structure to track sensor values
 *  Return: tau(float) - virtual impedance torque value
 */
//TODO: check that damping ratio is in the correct direction.
float actuateAngleLimits(Act_s *actx){
	static float bumperTorq=0.0;
	float tauK = 0; float tauB = 0;

	// apply unidirectional spring
	if ( actx->jointAngleDegrees < JOINT_MIN_SOFT_DEGREES ) {
		float thetaDelta = (JOINT_MIN_SOFT_DEGREES - actx->jointAngleDegrees);
		tauK = JOINT_SOFT_K * (thetaDelta)*(thetaDelta);
		tauB = -JOINT_SOFT_B * (actx->jointVelDegrees); //

		bumperTorq = tauK + tauB;

//		// apply unidirectional damper REMOVED, this slows it down more.
//		if ( actx->jointVelDegrees < 0.0) { //<
//
//			tauB = -JOINT_SOFT_B * (actx->jointVelDegrees);
//		} else
//		{
//			tauB = 0.0;
//		}


	} else if ( actx->jointAngleDegrees > JOINT_MAX_SOFT_DEGREES) {
		float thetaDelta = (JOINT_MAX_SOFT_DEGREES - actx->jointAngleDegrees);
		tauK = -JOINT_SOFT_K * (thetaDelta)*(thetaDelta);

		tauB = -JOINT_SOFT_B * (actx->jointVelDegrees);

		bumperTorq = tauK + tauB;


		//		// apply unidirectional damper
//		if ( actx->jointVelDegrees > 0.0) {
//
//			tauB = -JOINT_SOFT_B * actx->jointVelDegrees;
//
//		} else
//		{
//			tauB = 0.0;
//		}
	} else
	{
		tauK = 0.0;
		tauB = 0.0;
		bumperTorq = tauK + tauB;
	}


	return bumperTorq;
}

/*
 * Check current direction and provide baseline current to handle
 * the No load current requirement to get motor moving.
 * Param: desCurr(float) - 
 */
int32_t noLoadCurrent(float desCurr) {
	if (desCurr > 0) {
		return (int32_t) MOT_NOLOAD_CURRENT_POS;
	} else if (desCurr < 0) {
		return (int32_t) MOT_NOLOAD_CURRENT_NEG;
	} else {
		return (int32_t) 0;
	}
}


/*
 * Calculate required motor torque, based on joint torque. set motor torque
 * 			Motor Torque request, or maybe current
 * Param:	actx(struct act_s) - Actuator structure to track sensor values
 * Param:	tauDes(float_ - TODO:find out what this param is
 * 			TODO: find out what this is (old param?) ->tor_d, desired torque at joint [Nm]
 * Updates to Actuator structure:
 * 			actx->tauDes = tauDes(parameter);
 * 			actx->desiredCurrent = new desired current
 *
 */
void setMotorTorque(struct act_s *actx, float tauDes)
{
	//Saturation limit on Torque
	if (tauDes > ABS_TORQUE_LIMIT_INIT) {
		tauDes = ABS_TORQUE_LIMIT_INIT;
	} else if (tauDes < -ABS_TORQUE_LIMIT_INIT) {
		tauDes = -ABS_TORQUE_LIMIT_INIT;
	}

//	//Angle Limit bumpers
//	actx->tauDes = tauDes + actuateAngleLimits(actx);
	actx->tauMeas = actx->jointTorque;

	float N = actx->linkageMomentArm * N_SCREW;	// gear ratio

	//PID around motor torque
//	float tauC = getCompensatorPIDOutput(actx);

	// Custom Compensator Controller, todo: NOT STABLE DO NOT USE!!
	float tauC = getCompensatorCustomOutput(actx->tauMeas, actx->tauDes);


	//Angle Limit bumpers
	tauC = tauC + actuateAngleLimits(actx);


	// Feedforward term
	float tauFF = 0.0; 	// Not in use at the moment todo: figure out how to do this properly

	float tauCCombined = tauC + tauFF;

	rigid1.mn.genVar[6] = (int16_t) (tauCCombined*100.0);

	// motor current signal
	float Icalc = ( 1.0/(MOT_KT * N ) * tauCCombined  );	// Multiplier CURRENT_SCALAR_INIT to get to mA from Amps

	int32_t I = (int32_t) (Icalc * CURRENT_SCALAR_INIT );

	//DEBUG todo: check if you want to use this, or some other friction compensation methods
//	I = I + noLoadCurrent(I);	// Include current required to get moving

	//Saturate I to our current operational limits -- limit can be reduced by safetyShutoff() due to heating
	if (I > actx->currentOpLimit)
	{
		I = actx->currentOpLimit;
	} else if (I < -actx->currentOpLimit)
	{
		I = -actx->currentOpLimit;
	}

	actx->desiredCurrent = I; 	// demanded mA

	rigid1.mn.genVar[7] = (int16_t) (I);

	//DEBUG TURN OFF TEMPORARILY
	setMotorCurrent(actx->desiredCurrent, DEVICE_CHANNEL);	// send current command to comm buffer to Execute

	//variables used in cmd-rigid offset 5, for multipacket
	rigid1.mn.userVar[5] = actx->tauMeas*1000;	// x1000 is for float resolution in int32
	rigid1.mn.userVar[6] = actx->tauDes*1000;
}

/*
 * Control compensator based on system Identification and simulation
 * Input is system state
 * return:	torque command value to the motor driver
 */
float getCompensatorCustomOutput(float tauMeas, float tauRef)
{
	static float y[3] = {0, 0, 0};
	static float e[3] = {0, 0, 0};
	static int8_t k = 2;

	// shift previous values into new locations
//	e[k-2] = e[k-1];
//	e[k-1] = e[k];
//	y[k-2] = y[k-1];
//	y[k-1] = y[k];
//
	e[0] = e[1];
	e[1] = e[2];
	// update current state to new values
	e[2] = tauRef - tauMeas;

	y[0] = y[1];
	y[1] = y[2];

//	y[k] = 1.948*y[k-1] - 0.9483*y[k-2] + 1380.4*( e[k-1] - 0.9811*e[k-2] ) * ( e[k] - 1.982*e[k-1] + 0.9825*e[k-2] );	// Notch Filter, does not go below zero, but otherwise stable.
//	y[k] = 1.01831*y[k-1] - 0.01831*y[k-2] + 4006.6/1000.0*(e[k] - 1.995*e[k-1] + 0.9957*e[k-2]);		// This one drives downwards (negative), but does have positive and negative directionality

	// designed on 3/25/2019
//	y[k] = y[k-1] - 4.132e-15*y[k-2] + e[k] - 1.999*e[k-1] + e[k-2]*0.9993; // very very slow response
	// designed on 3/26/2019
//	y[k] = y[k-1] - 4.132e-15*y[k-2] + 3210*e[k] - 6350*e[k-1] + 3140*e[k-2]; // very noisy
//	y[k] = 1.712*y[k-1] - 0.7117*y[k-2] + 896.1*e[k] - 1780*e[k-1] + 884.2*e[k-2]; // shuts itself off
//	y[2] = 1.811*y[1] - 0.8106*y[0] + 112.9*e[2] - 223.2*e[1] + 110.3*e[0]; // shuts itself off
	y[2] = 1.088e-5*y[1] + 945.1*e[2] - 887.7*e[1]; // not stable, keep trying.

	rigid1.mn.genVar[8] = (int16_t) ( e[k] * 10);
	rigid1.mn.genVar[5] = (int16_t) ( y[k] * 10);
	// todo: consider saturating and tracking the saturated torque value inside of here?

	return ( y[2] );

}

/*
 * Control PID Controller
 * input:	Act_s structure
 * return:	desired torque signal to motor
 */
float getCompensatorPIDOutput(Act_s *actx)
{

	static float tauErrLast = 0.0, tauErrInt = 0.0;
	static int8_t tauErrIntWindup = 0;
	static float tauCCombined = 0.0, tauCOutput = 0.0;

	// Error is torque at the joint
	float tauErr = actx->tauDes - actx->tauMeas;		// [Nm]
	float tauErrDot = (tauErr - tauErrLast)*SECONDS;		// [Nm/s]
	tauErrLast = tauErr;

	// If there is no Integral Windup problem continue integrating error
	if (~tauErrIntWindup)
	{
		tauErrInt = tauErrInt + tauErr;				// [Nm]
	} else
	{
		//https://ocw.mit.edu/courses/aeronautics-and-astronautics/16-30-feedback-control-systems-fall-2010/lecture-notes/MIT16_30F10_lec23.pdf
		// try Antiwindup
		// 0) test, don't increase tauErrInt; // seems like it locks up in a direction
		// 1) test, set error to zero. // works okay, but kinda locks up at end limits
		// 2) test, don't test for sign of error, just if saturated. // locks up more, slower response
		// 3) test, set Ki = 0; this seems dangerous, rather know what this is doing.
		// 4) test, try a scalar on the error => tauErrInt = tauErrInt + errorKi * (tauCOutput - tauCCombined); // this seemed unstable, not driving the right direction
		tauErrInt = 0.0;	// this is reasonably stable.
	}

	//anti hunting for Integral term, if we're not moving, don't worry about it.
	if (fabs(actx->jointVel) < 0.1 )//&& fabs(actx->jointTorque) < 2.0)
	{
		tauErrInt = 0.0;
	}


	rigid1.mn.genVar[5] = (int16_t) (tauErrInt * 10.0);

	float tauC = tauErr*torqueKp + tauErrDot*torqueKd + tauErrInt*torqueKi;	// torq Compensator

	// Feedforward term
	float tauFF = 0.0; 	// Not in use at the moment todo: figure out how to do this properly

	tauCCombined = tauC + tauFF;

	//Saturation limit on Torque
	tauCOutput = tauCCombined;

	if (tauCCombined > ABS_TORQUE_LIMIT_INIT) {
		tauCOutput = ABS_TORQUE_LIMIT_INIT;
	} else if (tauCCombined < -ABS_TORQUE_LIMIT_INIT) {
		tauCOutput = -ABS_TORQUE_LIMIT_INIT;
	}

	// Clamp and turn off integral term if it's causing a torque saturation
//	if ( integralAntiWindup(tauErr, tauCCombined, tauCOutput) ){
//		tauErrInt = 0;
//	}
	tauErrIntWindup = integralAntiWindup(tauErr, tauCCombined, tauCOutput);


	return tauCOutput;
}


//Used for testing purposes. See state_machine
/*
 * Simple Biom controller
 * Param:	actx(struct act_s) - Actuator structure to track sensor values
 * Param:	thetaSet(float) - desired theta in DEGREES
 * Param:	k1(float) - impedance parameter
 * Param:	b(float) - impedance parameter
 * return: 	torD(float) -  desired torque
 */
float biomCalcImpedance(Act_s *actx,float k1, float b, float thetaSet)
{
	return k1 * (thetaSet - actx->jointAngleDegrees ) - b*actx->jointVelDegrees;

}

// Preferred Nomenclature Impedance Command
/*
 * Simple Impedance Controller
 * Param:	actx(struct act_s) - Actuator structure to track sensor values
 * Param:	thetaSet(float) - desired theta in DEGREES
 * Param:	k1(float) - impedance parameter
 * Param:	b(float) - impedance parameter
 * return: 	torD(float) -  desired torque
 */
float getImpedanceTorque(Act_s *actx, float k1, float b, float thetaSet)
{
	return k1 * (thetaSet - actx->jointAngleDegrees ) - b*actx->jointVelDegrees;
}

//Used for testing purposes. See state_machine
/*
 * Quadratic Impedance Controller
 * Param:	actx(struct act_s) - Actuator structure to track sensor values
 * Param:	thetaSet(float) - desired theta in DEGREES
 * Param:	k1(float) - impedance parameter
 * Param:	b(float) - impedance parameter
 * Param:	k2(float) - quadratic impedance parameter
 * return: 	torD(float) -  desired torque
 */
float getImpedanceTorqueQuadratic(Act_s *actx, float k1, float b, float thetaSet, float k2)
{
	float thetaDelta = thetaSet - actx->jointAngleDegrees;
	return k1 * ( thetaDelta ) + k2 * powf( thetaDelta, 2) - b*actx->jointVelDegrees;
}

/*
 *  Initialize Current controller on Execute, Set initial gains.
 */
void mitInitCurrentController(void) {

	act1.currentOpLimit = CURRENT_ENTRY_INIT;
	setControlMode(CTRL_CURRENT, DEVICE_CHANNEL);
	writeEx[DEVICE_CHANNEL].setpoint = 0;			// wasn't included in setControlMode, could be safe for init
	setControlGains(ACTRL_I_KP_INIT, ACTRL_I_KI_INIT, ACTRL_I_KD_INIT, 0, DEVICE_CHANNEL);
}

void mitInitOpenController(void) {

	act1.currentOpLimit = CURRENT_ENTRY_INIT;

	setControlMode(CTRL_OPEN, DEVICE_CHANNEL);
	writeEx[DEVICE_CHANNEL].setpoint = 0;
	setControlGains(0, 0, 0, 0, DEVICE_CHANNEL);


}

/*
 *  Updates the static variables tim
 *  Return: average(float) -er and polesState
 *
 *  Return: 0(int8_t)
 */
//int8_t findPoles(void) {
//	static uint32_t timer = 0;
//	static int8_t polesState = 0;
//
//	timer++;
//
//	switch(polesState) {
//		case 0:
//			//Disable FSM2:
//			disableActPackFSM2();
//			if(timer > 100)
//			{
//				polesState = 1;
//			}
//
//			return 0;
//
//		case 1:
//			//Send Find Poles command:
//
//			tx_cmd_calibration_mode_rw(TX_N_DEFAULT, CALIBRATION_FIND_POLES);
//			packAndSend(P_AND_S_DEFAULT, FLEXSEA_EXECUTE_1, mitDlegInfo, SEND_TO_SLAVE);
//			polesState = 2;
//			timer = 0;
//
//			return 0;
//
//		case 2:
//
//			if(timer >= 44*SECONDS)
//			{
//				//Enable FSM2, position controller
//				enableActPackFSM2();
//				return 1;
//			}
//			return 0;
//
//
//		default:
//
//			return 0;
//
//	}
//
//	return 0;
//}

/*
 *  TODO:find out what this function does and how its used
 *  Param:	val(int32_t) - current value given
 *  Return: average(float) - e rolling average of all previous and current values
 */
static float windowSmoothJoint(int32_t val) {
	#define JOINT_WINDOW_SIZE 5

	static int8_t index = -1;
	static float window[JOINT_WINDOW_SIZE];
	static float average = 0;


	index = (index + 1) % JOINT_WINDOW_SIZE;
	average -= window[index]/JOINT_WINDOW_SIZE;
	window[index] = (float) val;
	average += window[index]/JOINT_WINDOW_SIZE;

	return average;
}

/*
 *  TODO:find out what this function does and how its used
 *  Param:	val(int32_t) -
 *  Return: average(float) -
 */
static float windowSmoothAxial(float val) {
	#define AXIAL_WINDOW_SIZE 5

	static int8_t index = -1;
	static float window[AXIAL_WINDOW_SIZE];
	static float average = 0;


	index = (index + 1) % AXIAL_WINDOW_SIZE;
	average -= window[index]/AXIAL_WINDOW_SIZE;
	window[index] = val;
	average += window[index]/AXIAL_WINDOW_SIZE;

	return average;
}


/*
 * Set Motor torques open loop.  Used for System characterization.
 * Param:	actx(struct act_s) - Actuator structure to track sensor values
 * Param:	tauDes(float_ - TODO:find out what this param is
 * Updates to Actuator structure:
 * 			actx->tauDes = tauDes(parameter);
 * 			actx->desiredCurrent = new desired current
 *
 */
void setMotorTorqueOpenLoop(struct act_s *actx, float tauDes, int8_t motorControl)
{
	static int8_t isTransition = 0;
	static int8_t lastMotorControl = 0;

	if(lastMotorControl != motorControl)
	{
		isTransition = 1;
	}

//Angle Limit bumpers
	actx->tauDes = tauDes;// + actuateAngleLimits(actx);
	actx->tauMeas = actx->jointTorque;

	float N = actx->linkageMomentArm * N_SCREW;	// gear ratio

	// motor current signal
	float I = ( 1.0/(MOT_KT * N ) * actx->tauDes );		// [A]
	float V = (I * MOT_R) * voltageGain + ( MOT_KT_TOT * actx->motorVel * velGain) + (actx->motCurrDt * MOT_L * indGain ); // [V] this caused major problems? ==> ;

//	rigid1.mn.genVar[5] = (int16_t) ( (I * MOT_R)* voltageGain * CURRENT_SCALAR_INIT );
//	rigid1.mn.genVar[6] = (int16_t) ( ( MOT_KT_TOT * actx->motorVel ) * velGain * CURRENT_SCALAR_INIT );
//	rigid1.mn.genVar[7] = (int16_t) ( actx->motCurrDt * MOT_L * indGain * CURRENT_SCALAR_INIT );


	int32_t ImA = (int32_t) ( I * CURRENT_SCALAR_INIT );
	int32_t VmV = (int32_t) ( V * CURRENT_SCALAR_INIT );
//	I = I + noLoadCurrent(I);	// Include current required to get moving

	//Saturate I for our current operational limits -- limit can be reduced by safetyShutoff() due to heating
//	if (I > actx->currentOpLimit)
//	{
//		I = actx->currentOpLimit;
//	} else if (I < -actx->currentOpLimit)
//	{
//		I = -actx->currentOpLimit;
//	}

	switch(motorControl)
	{
		case 0: // current control
			if(isTransition)
			{
				mitInitCurrentController();
				isTransition = 0;
			}
//			setMotorCurrent( ImA , DEVICE_CHANNEL);	// send current [mA] command to comm buffer to Execute
			lastMotorControl = motorControl;
//			rigid1.mn.genVar[8] = (int16_t) ( ImA );
//			rigid1.mn.genVar[9] = (int16_t) ( VmV );
			actx->desiredCurrent = I;// + noLoadCurrent(I); 	// demanded mA

			break;
		case 1:	// voltage control
			if(isTransition)
			{
				mitInitOpenController();
				isTransition = 0;
			}
			setMotorVoltage( VmV, DEVICE_CHANNEL); // consider open volt control
			lastMotorControl = motorControl;
//			rigid1.mn.genVar[8] = (int16_t) ( ImA );
//			rigid1.mn.genVar[9] = (int16_t) ( VmV );
			break;
		default:
			break;
	}

}

/*
 * Compute where we are in frequency sweep
 * Param: 	omega		(float) - ____ in RADIANS PER SECOND
 * Param: 	t			(float) - time in SECONDS
 * Param:	amplitude	amplitude of input
 * Param:	dcBias		DC Bias to preload actuator drivetrain
 * Param:	noiseAmp	noise this is a noise scaling value to put ontop of signal
 * Return: torqueSetPoint(float) - torque adjusted by frequency of operation
 */
float torqueSystemIDFrequencySweep(float omega, float t, float amplitude, float dcBias, float noiseAmp){
	static float prevOmega = 0.0, prevAmp = 0., prevDC = 0., prevNoise = 0., lastSignal = 0.0;
	static float signal = 0.0, testSignal = 0;;
	static int8_t holdingOnTransition = 0;


	// only make a transition at a zero crossing and if the input has changed
	// Check for Transition
	if ( (prevOmega != omega) || (prevAmp != amplitude) || (prevDC != dcBias) || (prevNoise != noiseAmp) )
	{
		holdingOnTransition = 1;
		float testCase = fmodf(t, 2*M_PI/omega); //todo: this should be prevOmega, but I was having trouble getting out of 0 condition.

		//if we're at a transition point, now update all values
		if ( (testCase <= 0.002 ) && testSignal >= 0 )
		{
			holdingOnTransition = 0;

			prevOmega = omega;
			prevAmp = amplitude;
			prevDC = dcBias;
			prevNoise = noiseAmp;

		}

		rigid1.mn.genVar[5] = (int16_t) (testCase * 1000.0);
		rigid1.mn.genVar[6] = (int16_t) prevOmega*1000;
	}

	// if waiting for an elegant transition, keep going with the prev setting until ready to change over.
	if (holdingOnTransition == 1)
	{
		signal = prevDC + prevAmp * sinf(prevOmega * ( t  ) ) + prevNoise*( ((float)rand()) / ((float)RAND_MAX) );
	}
	else
	{
		signal = dcBias + amplitude * sinf(omega * ( t  ) ) + noiseAmp*( ((float)rand()) / ((float)RAND_MAX) );
	}

	testSignal = signal-lastSignal;
	lastSignal = signal;	// save to test condition in transition

	return ( signal );
}

/*
 * Step through series of data points to set torque
 * Pseudo Random Binary for systemID
 * Start when received signal
 * Return: torqueSetPoint(float) - TODO:find out what this value represents
 */
float torqueSystemIDPRBS(void)
{
	// Pseudo Random Binary for systemID
	static const int16_t inputDataSet[2047] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1};

	int8_t start = user_data_1.w[0];				// Turn it on.
	int16_t torqueAmplitude = user_data_1.w[1];		// amplitude of torque signal
	int8_t cyclingNumber = user_data_1.w[2];		// number of times to run through data set

	float torqueSetPoint = 0;

	int16_t cyclingPeriodLength =  (int16_t) ( sizeof(inputDataSet) / sizeof(inputDataSet[0]) -1);
	//	float pulseShiftingPeriod = 0.105;  // < 2*pi/(3*wc) = 2*pi/(3*20) = 0.105,unit of wc is Hz.
	static int16_t stepTimer = 0, arrayStep = 0;


	if (start == 1)
	{
		if (stepTimer <= cyclingNumber*cyclingPeriodLength)
		{
			if (arrayStep <= cyclingPeriodLength-1)
			{
				torqueSetPoint = (float) ( torqueAmplitude * inputDataSet[arrayStep] );
				//rigid1.mn.genVar[7] = (int16_t) (start); // record starting.
				arrayStep++;
			} else
			{
				arrayStep = 0;
				torqueSetPoint = 0.0;
			}
			stepTimer++;
		}
	}
	else
	{
		torqueSetPoint = 0.0;
		stepTimer = 0;
	}

	return torqueSetPoint;
}



//****************************************************************************
// Private Function(s)
//****************************************************************************

void setMotorNeutralPosition(struct act_s *actx)
{
	static int8_t tareState= -1;
	static uint16_t sample=0;
	uint16_t numSamples = 100;
	static int32_t motorEncReading = 0;

	float r = getLinkageMomentArm(&act1, act1.jointAngle, 1);

//	switch(tareState)
//		{
//			case -1:
//				//Tare the balance using average of numSamples readings
//				sample++;
//
//				if(sample <= numSamples)
//				{
//					motorEncReading =  motorEncReading + (int32_t)(*rigid1.ex.enc_ang/sample);
//				}
//				else
//				{
//					actx->motorPosNeutral = motorEncReading;
//					float r = getLinkageMomentArm(&act1, act1.jointAngle, 1);
//					tareState = 0;
//				}
//
//				break;
//
//			case 0:
//
//				break;
//		}

}

/*
 * Collect all sensor values and update the actuator structure.
 * Throws safety flags on Joint Angle, Joint Torque, since these functions look at transformed values.
 * Param:	actx(struct act_s) - Actuator structure to track sensor values
 */
void updateSensorValues(struct act_s *actx)
{
	getJointAngleKinematic(actx);

	actx->jointAngleDegrees = actx->jointAngle * DEG_PER_RAD;
	actx->jointVelDegrees = actx->jointVel * DEG_PER_RAD;

	actx->linkageMomentArm = getLinkageMomentArm(actx, actx->jointAngle, 0);

	actx->axialForce = getAxialForce(actx, zeroIt); //filtering happening inside function
	actx->jointTorque = getJointTorque(actx);

	updateJointTorqueRate(actx);

	actx->motorPosRaw = *rigid1.ex.enc_ang;

	actx->motorPos =  ( (float) *rigid1.ex.enc_ang ) * RAD_PER_MOTOR_CNT; //counts
	actx->motorVel =  ( (float) *rigid1.ex.enc_ang_vel ) * RAD_PER_MOTOR_CNT*SECONDS;	// rad/s TODO: check on motor encoder CPR, may not actually be 16384
	actx->motorAcc = rigid1.ex.mot_acc;	// rad/s/s

	actx->regTemp = rigid1.re.temp;
	actx->motTemp = 0; // REMOVED FOR NOISE ISSUES getMotorTempSensor();
	actx->motCurr = rigid1.ex.mot_current;
	actx->motCurrDt = getMotorCurrentDt(actx);

	actx->safetyFlag = getSafetyFlags(); //todo: I don't think this is in use anymore MC

	if(actx->regTemp > PCB_TEMP_LIMIT_INIT || actx->motTemp > MOTOR_TEMP_LIMIT_INIT)
	{
		isSafetyFlag = SAFETY_TEMP;
		isTempLimit = 1;
	} else {
		isTempLimit = 0;
	}
}
