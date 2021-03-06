
#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************
#include "walking_knee_ankle_state_machine.h"
#include <user-mn-MIT-DLeg.h>
#include "user-mn-MIT-EMG.h"
#include "free_ankle_EMG.h"
#include "spline_functions.h"
#include "torque_replay.h"
#include "nonlinear_stiff.h"

//****************************************************************************
// Definition(s):
//****************************************************************************
WalkingStateMachine kneeAnkleStateMachine;
WalkParams *ankleWalkParams, kneeWalkParams;
CubicSpline cubicSpline;
TorqueRep torqueRep;
NonLinearK nonLinearKParams;
float torque_traj_mscaled[TRAJ_SIZE]; // global variable

//NOTE: All of the damping values have been reduced by 1/10 due to controller

float splineTime = 100.0;

#ifdef BOARD_TYPE_FLEXSEA_MANAGE

//****************************************************************************
// Functions:
//****************************************************************************

void setSimpleAnkleFlatGroundFSM(Act_s *actx, WalkParams *ankleWalkParamx) {
	static int8_t isTransitioning = 0;
	static uint32_t timeInState = 0;
//	static int8_t passedStanceThresh = 0;
	static int8_t passedStanceThreshEst = 0;
	static int8_t lastPassedStanceThreshEst = 0;
	static float storedVirtualHardstopEngagementAngle;
	static float storedEstThetaDesAngle;

    kneeAnkleStateMachine.onEntrySmState = kneeAnkleStateMachine.currentState; // save the state on entry, assigned to last_currentState on exit
	actx->tauDes = 0.0;


	// Check for state change, then set isTransitioning flag
	if (kneeAnkleStateMachine.currentState == kneeAnkleStateMachine.lastSmState) {
		isTransitioning = 0;
		timeInState++;
	} else {
		// State transition, reset timers and set entry flag
		timeInState = 0;
		isTransitioning = 1;
	}


	    switch (kneeAnkleStateMachine.currentState) {

	        case STATE_IDLE: //-1
	        {
	        	//error handling here (should never be in STATE_IDLE by the time you get here)
	            break;
	        }
	        case STATE_INIT: //-2
	        {
	        	kneeAnkleStateMachine.currentState = STATE_EARLY_SWING;	// enter into early stance, this has stability. good idea for torque replay? might be better late swing

	            break;
	        }
	        case STATE_EARLY_STANCE: //0
	        { // check impedance mode in here - only stance state for torque replay (goes directly to early swing)

				if (isTransitioning) {
					ankleWalkParamx->timerInStance = 0;
					passedStanceThreshEst = 0;

					if( ankleWalkParamx->ankleGainsEst.thetaDes - actx->jointAngleDegrees < 0)
					{// If Dorsiflexed, we need to move spring neutral endpt to current location so we're not hammering the user
						//
						storedEstThetaDesAngle = ankleWalkParamx->ankleGainsEst.thetaDes;
						ankleWalkParamx->ankleGainsEst.thetaDes = actx->jointAngleDegrees;
					}
					if( actx->jointTorque > GENTLE_TOESTRIKE_TORQUE_THRESH )
					{// TODO Deal with a toe-strike, support it and move back into normal operation

						passedStanceThreshEst = 1; // Assume this is a toe strike and jump into mid-stance.
						kneeAnkleStateMachine.currentState = STATE_MID_STANCE;
//
//						storedEstThetaDesAngle = ankleWalkParamx->ankleGainsEst.thetaDes;
//
//						ankleWalkParamx->ankleGainsEst.kParam.thetaFinal = ankleWalkParamx->ankleGainsEst.thetaDes;
//						ankleWalkParamx->ankleGainsEst.kParam.thetaInit = actx->jointAngleDegrees;
//						ankleWalkParamx->ankleGainsEst.kParam.kCurrent = ankleWalkParamx->ankleGainsEst.k1;
//
//						updateStiffnessRampDTheta(actx, &ankleWalkParamx->ankleGainsEst.kParam);
//						ankleWalkParamx->ankleGainsEst.k1 = ankleWalkParamx->ankleGainsEst.kParam.kCurrent;
					}
				}
				ankleWalkParamx->timerInStance++;


				if( actx->jointAngleDegrees > ankleWalkParamx->virtualHardstopEngagementAngle && actx->jointVelDegrees < 0)
				{ // If plantarflexed, and now dorsiflexing (changed direction)
					passedStanceThreshEst = 1;

					if (passedStanceThreshEst != lastPassedStanceThreshEst)
					{ // just transitioned

						// Adjust spring constant, K, to be based around hardstopEngangementAngle instead of the stateTransition joint angle.
						// This prevents aggressive transition at heelstrike
						ankleWalkParamx->ankleGainsEst.kParam.thetaFinal = ankleWalkParamx->virtualHardstopEngagementAngle;
						ankleWalkParamx->ankleGainsEst.kParam.thetaInit = ankleWalkParamx->ankleGainsEst.thetaDes;
						ankleWalkParamx->ankleGainsEst.kParam.kCurrent = ankleWalkParamx->ankleGainsEst.k1;

						updateStiffnessRampDTheta(actx, &ankleWalkParamx->ankleGainsEst.kParam);
						ankleWalkParamx->ankleGainsEst.k1 = ankleWalkParamx->ankleGainsEst.kParam.kCurrent;
					}

				}

				actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsEst);

				/** Early Stance transition vectors **/
				if( actx->jointTorque > HARD_TOESTRIKE_TORQUE_THRESH )
				{// Deal with a toeStrike

					passedStanceThreshEst = 1; // Assume this is a toe strike and jump into mid-stance.
					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;

				} else if (passedStanceThreshEst && (actx->jointAngleDegrees <= ankleWalkParamx->virtualHardstopEngagementAngle) )
				{ // If passed through neutral once, and coming back, move to mid-stance with parallel spring

					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;

				} else if (actx->jointTorque > ankleWalkParamx->lspEngagementTorque)
				{// Transition occurs even if the early swing motion is not finished

					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;
				}



				break;
	        }


	        case STATE_MID_STANCE: //1
	        { // This is where parallel spring comes in
				if (isTransitioning)
				{
					// if we land at a new position (ie running or inclines) just go from there.
					storedVirtualHardstopEngagementAngle = ankleWalkParamx->virtualHardstopEngagementAngle;
					if ( actx->jointAngleDegrees < ankleWalkParamx->virtualHardstopEngagementAngle )
					{
						ankleWalkParamx->virtualHardstopEngagementAngle = actx->jointAngleDegrees;
					}

				}
				ankleWalkParamx->timerInStance++;

				updateAnkleVirtualHardstopTorque(actx, ankleWalkParamx);

				actx->tauDes = ankleWalkParamx->virtualHardstopTq + getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsMst);

				// Stance transition vectors, only go into next state. This is a stable place to be.
				// Transition occurs based on reaching torque threshold. (future: update this threshold based on speed)
				if (actx->jointTorque > ankleWalkParamx->lspEngagementTorque) {
					kneeAnkleStateMachine.currentState = STATE_LATE_STANCE_POWER;
					ankleWalkParamx->virtualHardstopEngagementAngle = storedVirtualHardstopEngagementAngle; // put back orig value.
				}

	        	break;
	        }
	        case STATE_LATE_STANCE_POWER: //2
	        {
				if (isTransitioning) {
					ankleWalkParamx->samplesInLSP = 0.0;
					ankleWalkParamx->lspEntryTq = actx->jointTorque;
				}

				ankleWalkParamx->timerInStance++;


				// This is the scaling factor for ramping into powered pushoff
				if (ankleWalkParamx->samplesInLSP < ankleWalkParamx->lstPGDelTics){
					ankleWalkParamx->samplesInLSP++;
				}

				updateAnkleVirtualHardstopTorque(actx, ankleWalkParamx);

				//Linear ramp to push off, pickup where hardstop leftoff, use stiffness ankleGainsLst to get us to target point.
				actx->tauDes = ankleWalkParamx->virtualHardstopTq + (ankleWalkParamx->samplesInLSP/ankleWalkParamx->lstPGDelTics) * getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsLst);  // drops off after zero when hardstop goes away

				//Late Stance Power transition vectors
				if ( (fabs(actx->jointTorque) < ANKLE_UNLOADED_TORQUE_THRESH) && (timeInState > LST_TO_ESW_DELAY ))
				{
					kneeAnkleStateMachine.currentState = STATE_EARLY_SWING;
					ankleWalkParamx->timerInStanceLast = ankleWalkParamx->timerInStance;

				}

	            break;
	        }

	        case STATE_EARLY_SWING: //3
	        {
				//Put anything you want to run ONCE during state entry.
				if (isTransitioning)
				{
					ankleWalkParamx->timerInSwing = 0;
					ankleWalkParamx->virtualHardstopTq = 0.0;
				}
				ankleWalkParamx->timerInSwing++;

				actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsEsw);

				if(timeInState >= ESW_TO_LSW_DELAY)
				{

					if(fabs(actx->tauMeas) >= fabs(GENTLE_HEELSTRIKE_TORQUE_THRESH) )
					{
						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;//STATE_LATE_SWING;
					}
				}

				break; // case STATE_EARLY_SWING
	        }

//			case STATE_LATE_SWING: //4
//			{
//				if (isTransitioning) {
//					ankleWalkParamx->transitionId = 0;
//				}
//				ankleWalkParamx->timerInSwing++;
//
//				actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsLsw);
//
//				//---------------------- LATE SWING TRANSITION VECTORS ----------------------//
//				if(timeInState > LSW_TO_EST_DELAY) {
//
//					// VECTOR (1): Late Swing -> Early Stance (hard heal strike) - Condition 1
//					if (actx->jointTorque > HARD_HEELSTRIKE_TORQUE_THRESH && actx->jointTorqueRate > HARD_HEELSTRIKE_TORQ_RATE_THRESH) {
//						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
//						ankleWalkParamx->transitionId = 1;
//					}
//					// VECTOR (1): Late Swing -> Early Stance (gentle heal strike) - Condition 2 -
//					else if (actx->jointTorqueRate > GENTLE_HEELSTRIKE_TORQ_RATE_THRESH) {
//						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
//						ankleWalkParamx->transitionId = 2;
//					}
//					// VECTOR (1): Late Swing -> Early Stance (toe strike) - Condition 3
//					else if (actx->jointAngleDegrees < HARD_TOESTRIKE_ANGLE_THRESH) {
//						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
//						ankleWalkParamx->transitionId = 3;
//					}
//
//				}
//					//------------------------- END OF TRANSITION VECTORS ------------------------//
//
//
//				break;
//			}
	        default:
	        {
	            //turn off control.
	            actx->tauDes = 0;
				ankleWalkParamx->timerInSwing = 0;
				ankleWalkParamx->timerInStance = 0;

	            kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
	            break;
	        }
	    }

	    //update last state in preparation for next loop
	    kneeAnkleStateMachine.lastSmState = kneeAnkleStateMachine.onEntrySmState;

}

/** Impedance Control Level-ground Walking FSM
	Ankle is SLAVE:	determines walking state. Sends out the state to the Knee
	Knee is MASTER: receives kneeAnkleStateMachine.currentState updates from
					the Ankle actuator, and changes impedance settings accordingly
	Param: actx(Act_s) - Actuator structure to track sensor values
	Param: actx(Act_s)

	ptorqueDes pointer to float meant to be updated with desired torque TODO:find out what this is
*/

// torque replay function
void setAnkleTorqueReplay(Act_s *actx, WalkParams *ankleWalkParamx){
	/*
	 * Uses standard Walking FSM, in early-stance make transition to Torque Replay mode
	 * If previous stance is above a set threshold. Basic walking speed is 1.25m/s,
	 * speed factor should allow us to scale faster than that, not slower.
	 */
	static int8_t isTransitioning = 0;
	static uint32_t timeInState = 0;
	static int8_t passedStanceThreshEst = 0;
	static int8_t lastPassedStanceThreshEst = 0;
	static float storedVirtualHardstopEngagementAngle;
	static float storedEstThetaDesAngle;

	if(actx->resetStaticVariables)
	{
		isTransitioning = 0;
		timeInState = 0;
		passedStanceThreshEst = 0;
		lastPassedStanceThreshEst = 0;
		storedVirtualHardstopEngagementAngle = 0.0;
		storedEstThetaDesAngle = 0.0;
	}

	kneeAnkleStateMachine.onEntrySmState = kneeAnkleStateMachine.currentState; // save the state on entry, assigned to last_currentState on exit
	actx->tauDes = 0.0;


	// Check for state change, then set isTransitioning flag
	if (kneeAnkleStateMachine.currentState == kneeAnkleStateMachine.lastSmState) {
		isTransitioning = 0;
		timeInState++;
	} else {
		// State transition, reset timers and set entry flag
		timeInState = 0;
		isTransitioning = 1;
	}


		switch (kneeAnkleStateMachine.currentState) {

			case STATE_IDLE: //-1
			{
				//error handling here (should never be in STATE_IDLE by the time you get here)
				break;
			}
			case STATE_INIT: //-2
			{
				kneeAnkleStateMachine.currentState = STATE_EARLY_SWING;	// enter into early stance, this has stability. good idea for torque replay? might be better late swing

				torqueRep.time_stance = 0.0;
				torqueRep.standard_stance_period = 555.0; 	// [ms]
				torqueRep.previous_stance_period = 555.0; 	// [ms]
				torqueRep.time_swing = 0.0;
				torqueRep.standard_swing_period = 400.0;  	// [ms]
				torqueRep.previous_swing_period = 400.0;  	// [ms]
				torqueRep.torqueScalingFactor = 0.1; 		// adjust maximum torque output
				torqueRep.entry_replay = 0;					// verify if we were already running torque replay
				torqueRep.begin = 0;

				for(int i=0; i<TRAJ_SIZE; i++)
				{
					torque_traj_mscaled[i] = torque_traj[i] + ((USER_MASS - 70.0)*massGains[i]);
				}

				break;
			}
			case STATE_EARLY_STANCE: //0
			{ // check impedance mode in here - only stance state for torque replay (goes directly to early swing)


				if (isTransitioning) {
					ankleWalkParamx->timerInStance = 0;
					passedStanceThreshEst = 0;

					if( ankleWalkParamx->ankleGainsEst.thetaDes - actx->jointAngleDegrees < 0)
					{// If Dorsiflexed, we need to move spring neutral endpt to current location so we're not hammering the user
						//
						storedEstThetaDesAngle = ankleWalkParamx->ankleGainsEst.thetaDes;
						ankleWalkParamx->ankleGainsEst.thetaDes = actx->jointAngleDegrees;
					}
					else if( actx->jointTorque > GENTLE_TOESTRIKE_TORQUE_THRESH )
					{// TODO Deal with a toe-strike, support it and move back into normal operation
						storedEstThetaDesAngle = ankleWalkParamx->ankleGainsEst.thetaDes;

						ankleWalkParamx->ankleGainsEst.kParam.thetaFinal = ankleWalkParamx->ankleGainsEst.thetaDes;
						ankleWalkParamx->ankleGainsEst.kParam.thetaInit = actx->jointAngleDegrees;
						ankleWalkParamx->ankleGainsEst.kParam.kCurrent = ankleWalkParamx->ankleGainsEst.k1;

						updateStiffnessRampDTheta(actx, &ankleWalkParamx->ankleGainsEst.kParam);
						ankleWalkParamx->ankleGainsEst.k1 = ankleWalkParamx->ankleGainsEst.kParam.kCurrent;
					}
				}
				ankleWalkParamx->timerInStance++;


				if( actx->jointAngleDegrees > ankleWalkParamx->virtualHardstopEngagementAngle && actx->jointVelDegrees < 0)
				{ // If plantarflexed, and now dorsiflexing (changed direction)
					passedStanceThreshEst = 1;

					if (passedStanceThreshEst != lastPassedStanceThreshEst)
					{ // just transitioned

						// Adjust spring constant, K, to be based around hardstopEngangementAngle instead of the stateTransition joint angle.
						// This prevents aggressive transition at heelstrike
						ankleWalkParamx->ankleGainsEst.kParam.thetaFinal = ankleWalkParamx->virtualHardstopEngagementAngle;
						ankleWalkParamx->ankleGainsEst.kParam.thetaInit = ankleWalkParamx->ankleGainsEst.thetaDes;
						ankleWalkParamx->ankleGainsEst.kParam.kCurrent = ankleWalkParamx->ankleGainsEst.k1;

						updateStiffnessRampDTheta(actx, &ankleWalkParamx->ankleGainsEst.kParam);
						ankleWalkParamx->ankleGainsEst.k1 = ankleWalkParamx->ankleGainsEst.kParam.kCurrent;
					}

				}

				actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsEst);

				/** Early Stance transition vectors **/
				if( actx->jointTorque > HARD_TOESTRIKE_TORQUE_THRESH )
				{// Deal with a toeStrike

					passedStanceThreshEst = 1; // Assume this is a toe strike and jump into mid-stance.
					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;

				} else if (passedStanceThreshEst && (actx->jointAngleDegrees <= ankleWalkParamx->virtualHardstopEngagementAngle) )
				{ // If passed through neutral once, and coming back, move to mid-stance with parallel spring

					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;

				} else if (actx->jointTorque > ankleWalkParamx->lspEngagementTorque)
				{// Transition occurs even if the early swing motion is not finished

					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;
				}

				break;
			}


			case STATE_MID_STANCE: //1
			{ // This is where parallel spring comes in

				if(checkImpedanceMode(&torqueRep))
				{
					if (isTransitioning)
					{
						// if we land at a new position (ie running or inclines) just go from there.
						storedVirtualHardstopEngagementAngle = ankleWalkParamx->virtualHardstopEngagementAngle;
						if ( actx->jointAngleDegrees < ankleWalkParamx->virtualHardstopEngagementAngle )
						{
							ankleWalkParamx->virtualHardstopEngagementAngle = actx->jointAngleDegrees;
						}

					}
					ankleWalkParamx->timerInStance++;

					updateAnkleVirtualHardstopTorque(actx, ankleWalkParamx);

					actx->tauDes = ankleWalkParamx->virtualHardstopTq + getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsMst);

					// Stance transition vectors, only go into next state. This is a stable place to be.
					// Transition occurs based on reaching torque threshold. (future: update this threshold based on speed)
					if (actx->jointTorque > ankleWalkParamx->lspEngagementTorque) {
						kneeAnkleStateMachine.currentState = STATE_LATE_STANCE_POWER;
						ankleWalkParamx->virtualHardstopEngagementAngle = storedVirtualHardstopEngagementAngle; // put back orig value.
					}
				}
				else
				{
					kneeAnkleStateMachine.currentState = STATE_TORQUE_REPLAY;
				}


				break;
			}
			case STATE_LATE_STANCE_POWER: //2
			{
				if (isTransitioning) {
					ankleWalkParamx->samplesInLSP = 0.0;
					ankleWalkParamx->lspEntryTq = actx->jointTorque;
				}

				ankleWalkParamx->timerInStance++;


				// This is the scaling factor for ramping into powered pushoff
				if (ankleWalkParamx->samplesInLSP < ankleWalkParamx->lstPGDelTics){
					ankleWalkParamx->samplesInLSP++;
				}

				updateAnkleVirtualHardstopTorque(actx, ankleWalkParamx);

				//Linear ramp to push off, pickup where hardstop leftoff, use stiffness ankleGainsLst to get us to target point.
				actx->tauDes = ankleWalkParamx->virtualHardstopTq + (ankleWalkParamx->samplesInLSP/ankleWalkParamx->lstPGDelTics) * getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsLst);  // drops off after zero when hardstop goes away

				//Late Stance Power transition vectors
				if ( (fabs(actx->jointTorque) < ANKLE_UNLOADED_TORQUE_THRESH) && (timeInState > LST_TO_ESW_DELAY )) //&& (actx->jointAngleDegrees >=  ankleGainsLst.thetaDes -1.0) ) {	// not sure we need the timeInState? what's the point? just maker sure it's kicking?
				{
					kneeAnkleStateMachine.currentState = STATE_EARLY_SWING;
					ankleWalkParamx->timerInStanceLast = ankleWalkParamx->timerInStance;

				}

				break;
			}

			case STATE_EARLY_SWING: //3
			{
				//Put anything you want to run ONCE during state entry.
				if (isTransitioning)
				{
					ankleWalkParamx->timerInSwing = 0;
					ankleWalkParamx->virtualHardstopTq = 0.0;
				}
				ankleWalkParamx->timerInSwing++;

				actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsEsw);
				torqueRep.time_swing++;

				if(timeInState >= ESW_TO_LSW_DELAY)
				{

					if(fabs(actx->tauMeas) >= fabs(GENTLE_HEELSTRIKE_TORQUE_THRESH) )
					{
						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;//STATE_LATE_SWING;
						torqueRep.previous_swing_period = torqueRep.time_swing;
						torqueRep.time_swing = 0;
					}
				}

				break; // case STATE_EARLY_SWING
			}

//			case STATE_LATE_SWING: //4
//			{
//				if (isTransitioning) {
//					ankleWalkParamx->transitionId = 0;
//				}
//				ankleWalkParamx->timerInSwing++;
//
//				actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsLsw);
//
//				//---------------------- LATE SWING TRANSITION VECTORS ----------------------//
//				if(timeInState > LSW_TO_EST_DELAY) {
//
//					 // VECTOR (1): Late Swing -> Early Stance (hard heal strike) - Condition 1
//					if (actx->jointTorque > HARD_HEELSTRIKE_TORQUE_THRESH && actx->jointTorqueRate > HARD_HEELSTRIKE_TORQ_RATE_THRESH) {
//						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
//						ankleWalkParamx->transitionId = 1;
//					}
//					// VECTOR (1): Late Swing -> Early Stance (gentle heal strike) - Condition 2 -
//					else if (actx->jointTorqueRate > GENTLE_HEELSTRIKE_TORQ_RATE_THRESH) {
//						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
//						ankleWalkParamx->transitionId = 2;
//					}
//					// VECTOR (1): Late Swing -> Early Stance (toe strike) - Condition 3
//					else if (actx->jointAngleDegrees < HARD_TOESTRIKE_ANGLE_THRESH) {
//						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
//						ankleWalkParamx->transitionId = 3;
//					}
//
//				}
//					//------------------------- END OF TRANSITION VECTORS ------------------------//
//
//
//				break;
//			}

			case STATE_TORQUE_REPLAY: // 9
			{
				torqueRep.entry_replay = 1;	// let's us know we're in torqueReplay mode

				actx->tauDes = torqueTracking(&torqueRep);	//this is where the magic happens, generate torque profile

				torqueRep.time_stance++;	// stance timer

				//Late Stance Power transition vectors
				if ( (fabs(actx->jointTorque) < ANKLE_UNLOADED_TORQUE_THRESH) && (timeInState > LST_TO_ESW_DELAY )) //&& (actx->jointAngleDegrees >=  ankleGainsLst.thetaDes -1.0) ) {	// not sure we need the timeInState? what's the point? just maker sure it's kicking?
				{
					kneeAnkleStateMachine.currentState = STATE_EARLY_SWING;
					//ankleWalkParamx->timerInStanceLast = ankleWalkParamx->timerInStance;

					torqueRep.previous_stance_period = torqueRep.time_stance;
					torqueRep.time_stance = 0.0;
				}

				break;
			}

			default:
			{
				//turn off control.
				actx->tauDes = 0;
				ankleWalkParamx->timerInSwing = 0;
				ankleWalkParamx->timerInStance = 0;

				kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
				break;
			}
		}

		//update last state in preparation for next loop
		kneeAnkleStateMachine.lastSmState = kneeAnkleStateMachine.onEntrySmState;

}

// non-linear stiffness Walking State Machine function
void setAnkleNonLinearStiffWalkingFSM(Act_s *actx, WalkParams *ankleWalkParamx, NonLinearK *nonLinearKParamx){
	/*
	 * Uses standard Walking FSM, in early-stance make transition to Torque Replay mode
	 * If previous stance is above a set threshold. Basic walking speed is 1.25m/s,
	 * speed factor should allow us to scale faster than that, not slower.
	 */
	static int8_t isTransitioning = 0;
	static uint32_t timeInState = 0;
	static int8_t passedStanceThreshEst = 0;
	static int8_t lastPassedStanceThreshEst = 0;
	static float storedEstThetaDesAngle;

	kneeAnkleStateMachine.onEntrySmState = kneeAnkleStateMachine.currentState; // save the state on entry, assigned to last_currentState on exit
	actx->tauDes = 0.0;


	// Check for state change, then set isTransitioning flag
	if (kneeAnkleStateMachine.currentState == kneeAnkleStateMachine.lastSmState) {
		isTransitioning = 0;
		timeInState++;
	} else {
		// State transition, reset timers and set entry flag
		timeInState = 0;
		isTransitioning = 1;
	}


		switch (kneeAnkleStateMachine.currentState) {

			case STATE_IDLE: //-1
			{
				//error handling here (should never be in STATE_IDLE by the time you get here)
				break;
			}
			case STATE_INIT: //-2
			{
				// non-linear stiffness variable init, if needed
				nonLinearKParamx->ascAngleIndex	= 0;
				nonLinearKParamx->descAngleIndex = NONL_TRAJ_MAX_INDEX;
				nonLinearKParamx->earlyLateFlag	= 0;
				nonLinearKParamx->stiffnessCurrentVal =ankleWalkParamx->ankleGainsNonLinear.k1;

				kneeAnkleStateMachine.currentState = STATE_EARLY_SWING;	// enter into early stance, this has stability. good idea for torque replay? might be better late swing

				break;
			}
			case STATE_EARLY_STANCE: //0
			{ // check impedance mode in here - only stance state for torque replay (goes directly to early swing)


				/*
				updateAnkleVirtualHardstopTorque(actx, ankleWalkParamx);
				actx->tauDes = ankleWalkParamx->virtualHardstopTq + getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsEst);

				if (JNT_ORIENT*actx->jointAngleDegrees > ankleWalkParamx->virtualHardstopEngagementAngle)
				{
					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;
					passedStanceThresh = 1;
				}
				*/
				if (isTransitioning) {
					ankleWalkParamx->timerInStance = 0;
					passedStanceThreshEst = 0;

					if( ankleWalkParamx->ankleGainsEst.thetaDes - actx->jointAngleDegrees < 0)
					{// If Dorsiflexed, we need to move spring neutral endpt to current location so we're not hammering the user
						//
						storedEstThetaDesAngle = ankleWalkParamx->ankleGainsEst.thetaDes;
						ankleWalkParamx->ankleGainsEst.thetaDes = actx->jointAngleDegrees;
					}
					else if( actx->jointTorque > GENTLE_TOESTRIKE_TORQUE_THRESH )
					{// TODO Deal with a toe-strike, support it and move back into normal operation
						storedEstThetaDesAngle = ankleWalkParamx->ankleGainsEst.thetaDes;

						ankleWalkParamx->ankleGainsEst.kParam.thetaFinal = ankleWalkParamx->ankleGainsEst.thetaDes;
						ankleWalkParamx->ankleGainsEst.kParam.thetaInit = actx->jointAngleDegrees;
						ankleWalkParamx->ankleGainsEst.kParam.kCurrent = ankleWalkParamx->ankleGainsEst.k1;

						updateStiffnessRampDTheta(actx, &ankleWalkParamx->ankleGainsEst.kParam);
						ankleWalkParamx->ankleGainsEst.k1 = ankleWalkParamx->ankleGainsEst.kParam.kCurrent;
					}
				}
				ankleWalkParamx->timerInStance++;


				if( actx->jointAngleDegrees > ankleWalkParamx->virtualHardstopEngagementAngle && actx->jointVelDegrees < 0)
				{ // If plantarflexed, and now dorsiflexing (changed direction)
					passedStanceThreshEst = 1;

					if (passedStanceThreshEst != lastPassedStanceThreshEst)
					{ // just transitioned

						// Adjust spring constant, K, to be based around hardstopEngangementAngle instead of the stateTransition joint angle.
						// This prevents aggressive transition at heelstrike
						ankleWalkParamx->ankleGainsEst.kParam.thetaFinal = ankleWalkParamx->virtualHardstopEngagementAngle;
						ankleWalkParamx->ankleGainsEst.kParam.thetaInit = ankleWalkParamx->ankleGainsEst.thetaDes;
						ankleWalkParamx->ankleGainsEst.kParam.kCurrent = ankleWalkParamx->ankleGainsEst.k1;

						updateStiffnessRampDTheta(actx, &ankleWalkParamx->ankleGainsEst.kParam);
						ankleWalkParamx->ankleGainsEst.k1 = ankleWalkParamx->ankleGainsEst.kParam.kCurrent;
					}

				}

				actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsEst);

				/** Early Stance transition vectors **/
				if( actx->jointTorque > HARD_TOESTRIKE_TORQUE_THRESH )
				{// Deal with a toeStrike

					passedStanceThreshEst = 1; // Assume this is a toe strike and jump into mid-stance.
					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;

				} else if (passedStanceThreshEst && (actx->jointAngleDegrees <= ankleWalkParamx->virtualHardstopEngagementAngle) )
				{ // If passed through neutral once, and coming back, move to mid-stance with parallel spring

					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;

				} else if (actx->jointTorque > ankleWalkParamx->lspEngagementTorque)
				{// Transition occurs even if the early swing motion is not finished

					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;
				}

				break;
			}


			case STATE_MID_STANCE: //1
			{ // This is where parallel spring comes in

				kneeAnkleStateMachine.currentState = STATE_NONLINEAR_STIFF;


				break;
			}
			case STATE_LATE_STANCE_POWER: //2
			{
				if (isTransitioning) {
					ankleWalkParamx->samplesInLSP = 0.0;
					ankleWalkParamx->lspEntryTq = actx->jointTorque;
				}

				ankleWalkParamx->timerInStance++;


				// This is the scaling factor for ramping into powered pushoff
				if (ankleWalkParamx->samplesInLSP < ankleWalkParamx->lstPGDelTics){
					ankleWalkParamx->samplesInLSP++;
				}

				updateAnkleVirtualHardstopTorque(actx, ankleWalkParamx);

				//Linear ramp to push off, pickup where hardstop leftoff, use stiffness ankleGainsLst to get us to target point.
				actx->tauDes = ankleWalkParamx->virtualHardstopTq + (ankleWalkParamx->samplesInLSP/ankleWalkParamx->lstPGDelTics) * getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsLst);  // drops off after zero when hardstop goes away

				//Late Stance Power transition vectors
				if ( (fabs(actx->jointTorque) < ANKLE_UNLOADED_TORQUE_THRESH) && (timeInState > LST_TO_ESW_DELAY )) //&& (actx->jointAngleDegrees >=  ankleGainsLst.thetaDes -1.0) ) {	// not sure we need the timeInState? what's the point? just maker sure it's kicking?
				{
					kneeAnkleStateMachine.currentState = STATE_EARLY_SWING;
					ankleWalkParamx->timerInStanceLast = ankleWalkParamx->timerInStance;

				}

				break;
			}

			case STATE_EARLY_SWING: //3
			{
				//Put anything you want to run ONCE during state entry.
				if (isTransitioning)
				{
					ankleWalkParamx->timerInSwing = 0;
					ankleWalkParamx->virtualHardstopTq = 0.0;
				}
				ankleWalkParamx->timerInSwing++;

				actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsEsw);
				torqueRep.time_swing++;

				if(timeInState >= ESW_TO_LSW_DELAY)
				{
//					if( actx->tauMeas >= HARD_TOESTRIKE_TORQUE_THRESH )
//					{ // If hammering the toe, jump into pushoff?
//						kneeAnkleStateMachine.currentState = STATE_MID_STANCE;
//					}
					if(fabs(actx->tauMeas) >= fabs(GENTLE_HEELSTRIKE_TORQUE_THRESH) )
					{
						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;//STATE_LATE_SWING;
						torqueRep.previous_swing_period = torqueRep.time_swing;
						torqueRep.time_swing = 0;
					}
				}

				break; // case STATE_EARLY_SWING
			}

//			case STATE_LATE_SWING: //4
//			{
//				if (isTransitioning) {
//					ankleWalkParamx->transitionId = 0;
//				}
//				ankleWalkParamx->timerInSwing++;
//
//				actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsLsw);
//
//				//---------------------- LATE SWING TRANSITION VECTORS ----------------------//
//				if(timeInState > LSW_TO_EST_DELAY) {
//
//					// VECTOR (1): Late Swing -> Early Stance (hard heal strike) - Condition 1
//					if (actx->jointTorque > HARD_HEELSTRIKE_TORQUE_THRESH && actx->jointTorqueRate > HARD_HEELSTRIKE_TORQ_RATE_THRESH) {
//						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
//						ankleWalkParamx->transitionId = 1;
//					}
//					// VECTOR (1): Late Swing -> Early Stance (gentle heal strike) - Condition 2 -
//					else if (actx->jointTorqueRate > GENTLE_HEELSTRIKE_TORQ_RATE_THRESH) {
//						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
//						ankleWalkParamx->transitionId = 2;
//					}
//					// VECTOR (1): Late Swing -> Early Stance (toe strike) - Condition 3
//					else if (actx->jointAngleDegrees < HARD_TOESTRIKE_ANGLE_THRESH) {
//						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
//						ankleWalkParamx->transitionId = 3;
//					}
//
//				}
//					//------------------------- END OF TRANSITION VECTORS ------------------------//
//
//
//				break;
//			}

			case STATE_TORQUE_REPLAY: // 9
			{
				torqueRep.entry_replay = 1;	// let's us know we're in torqueReplay mode

				actx->tauDes = torqueTracking(&torqueRep);	//this is where the magic happens, generate torque profile

				torqueRep.time_stance++;	// stance timer

				//Late Stance Power transition vectors
				if ( (fabs(actx->jointTorque) < ANKLE_UNLOADED_TORQUE_THRESH) && (timeInState > LST_TO_ESW_DELAY )) //&& (actx->jointAngleDegrees >=  ankleGainsLst.thetaDes -1.0) ) {	// not sure we need the timeInState? what's the point? just maker sure it's kicking?
				{
					kneeAnkleStateMachine.currentState = STATE_EARLY_SWING;
					//ankleWalkParamx->timerInStanceLast = ankleWalkParamx->timerInStance;

					torqueRep.previous_stance_period = torqueRep.time_stance;
					torqueRep.time_stance = 0.0;
				}

				break;
			}

			case STATE_NONLINEAR_STIFF: // 10
			{
				// search in lookup tables function here

				//TODO If this doesn't work, try outputting torque value directly.
				//NOTE: STATE CHANGE HAPPENS INSIDE THIS FUNCTION
				//ankleWalkParamx->ankleGainsNonLinear.k1 = getNonlinearStiffness(actx, ankleWalkParamx, &kneeAnkleStateMachine, nonLinearKParamx);
				//actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsNonLinear);

				//initialize our current state.
				nonLinearKParamx->currentWalkingState = kneeAnkleStateMachine.currentState;

				if ( (actx->jointAngleDegrees > ascAngle[0]) && (nonLinearKParamx->earlyLateFlag == 0) )
				{ // try to prevent over plantarflexion during thist state.
					ankleWalkParamx->ankleGainsNonLinear.thetaDes 	= ascAngle[0];

					actx->tauDes = getImpedanceTorqueParams(actx, &ankleWalkParamx->ankleGainsNonLinear);

				} else
				{
					actx->tauDes = getNonlinearStiffness(actx, ankleWalkParamx, &kneeAnkleStateMachine, nonLinearKParamx);

					//update current state from above function.
					kneeAnkleStateMachine.currentState = nonLinearKParamx->currentWalkingState;
				}
				break;
			}

			default:
			{
				//turn off control.
				actx->tauDes = 0;
				ankleWalkParamx->timerInSwing = 0;
				ankleWalkParamx->timerInStance = 0;

				kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
				break;
			}
		}

		//update last state in preparation for next loop
		kneeAnkleStateMachine.lastSmState = kneeAnkleStateMachine.onEntrySmState;

}


void setAnklePassiveDeviceFSM(Act_s *actx){

}

//
/** This ramps down the Stiffness of early stance K, picking up the user and bringing them up to ankleGainsEst.thetaDes
    NOTE/TODO: Hugh may prefer much more stiffness here, this ramps down teh stiffness.
	Param: actx(Act_s) - Actuator structure to track sensor values
	Param: wParams(WalkParams) - Parameters relating to walking states
*/
void updateImpedanceParams(Act_s *actx, WalkParams *wParams) {

	wParams->scaleFactor = wParams->scaleFactor * wParams->earlyStanceDecayConstant;

	wParams->ankleGainsEst.k1 = wParams->earlyStanceKF + wParams->scaleFactor * (wParams->earlyStanceK0 - wParams->earlyStanceKF);

    if (actx->jointAngleDegrees < wParams->ankleGainsEst.thetaDes) {
    	wParams->ankleGainsEst.thetaDes = actx->jointAngleDegrees;
    }
}

/** TODO: Find out what this does
	Param: actx(Act_s) - Actuator structure to track sensor values
	Param: wParams(WalkParams) - Parameters relating to walking states
*/
void updateAnkleVirtualHardstopTorque(Act_s *actx, WalkParams *wParams) {

	if (JNT_ORIENT*actx->jointAngleDegrees > wParams->virtualHardstopEngagementAngle) {
		wParams->virtualHardstopTq = wParams->virtualHardstopK * ((JNT_ORIENT * actx->jointAngleDegrees) - wParams->virtualHardstopEngagementAngle);
	} else {
		wParams->virtualHardstopTq = 0.0;
	}
}

// torque replay functions
int8_t checkImpedanceMode(TorqueRep *torqueRep){
	// include user writing for setting impedance mode

	if(( torqueRep->previous_swing_period <= torqueRep->standard_swing_period ) && torqueRep->begin)
	{ // Up to speed and permission = torque replay
		return 0;
	}
	else if( (torqueRep->previous_swing_period >= IMPEDANCE_MODE_THRESHOLD) && (torqueRep->entry_replay) )
	{ // Check that you were in TorqueReplay, but slowed down so turn off
		torqueRep->entry_replay = 0;
		return 1;
	}
	else
	{ // impedance mode
		torqueRep->entry_replay = 0;
		return 1;
	}

}


float torqueTracking(TorqueRep *torqueRep)
{	// Replay a torque profile
	if(torqueRep->previous_stance_period > torqueRep->standard_stance_period)
	{
		torqueRep->previous_stance_period = torqueRep->standard_stance_period;
	}

	torqueRep->speedFactor = (1.0 - ( torqueRep->previous_stance_period / torqueRep->standard_stance_period ) )*100.0;
	torqueRep->percent = torqueRep->time_stance / torqueRep->previous_stance_period;

	if(torqueRep->percent > 1.0)
	{
		torqueRep->percent = 1.0;
	}

	torqueRep->index = round( torqueRep->percent * (float) TRAJ_SIZE );

	if( torqueRep->index > MAX_TRAJ_INDEX )
	{
		torqueRep->index = MAX_TRAJ_INDEX;
	}

	torqueRep->tauDes = torqueRep->torqueScalingFactor * ( torque_traj_mscaled[torqueRep->index] + ( torqueRep->speedFactor*speedGains[torqueRep->index] ) );

	torqueRep->tauDesPlot = ( torque_traj_mscaled[torqueRep->index] + ( torqueRep->speedFactor*speedGains[torqueRep->index] ) ); // full torque profile

	return torqueRep->tauDes;
}


void setTorqueAnklePassive(Act_s *actx, WalkParams *wParams)
{ // simulate a passive actuator, just a damped spring
	actx->tauDes = getImpedanceTorque(actx, wParams->virtualHardstopK, wParams->virtualHardstopB, wParams->virtualHardstopEngagementAngle);
}

void setTorqueQuasiPassive(Act_s *actx, WalkParams *wParams)
{ // simulate a quasi-passive actuator, two springs, one for heelstrike, one for mid-stance.
	actx->tauDes = 0; //
}

void updateStiffnessRampDt(RampParam *rampParamx)
{ // Calc new impedance K to ramp into Parallel Spring big K
	rampParamx->kInit = rampParamx->kCurrent;

	if (rampParamx->runningTimer < rampParamx->deltaTime)
	{
		rampParamx->runningTimer++;
	}

	rampParamx->kCurrent = (rampParamx->kFinal - rampParamx->kInit)/rampParamx->deltaTime * rampParamx->runningTimer + rampParamx->kInit;
}

void updateStiffnessRampDTheta(Act_s *actx, RampParam *rampParamx)
{ // Update impedance Stiffness K to match Force with new ThetaSetpt
	rampParamx->kInit = rampParamx->kCurrent;
	rampParamx->thetaInit = rampParamx->thetaCurrent;
	rampParamx->kCurrent = rampParamx->kInit * ( actx->jointAngleDegrees -  rampParamx->thetaInit)
											/ ( actx->jointAngleDegrees -  rampParamx->thetaFinal );
}


float getNonlinearStiffness(Act_s *actx, WalkParams *wParams, WalkingStateMachine *stateMachine, NonLinearK *nonLinearKParamx)
{	// lookup stiffness based on joint angle, Joint orientation is Dorsi is (-) angle

	// reset these indices for each time.
	nonLinearKParamx->ascAngleIndex = 0;
	nonLinearKParamx->descAngleIndex = NONL_TRAJ_MAX_INDEX;

	if(nonLinearKParamx->earlyLateFlag == 0)
	{ // Mid-Stance Controlled Dorsiflexion, going into Powered Plantarflexion,
		for(int16_t i = 0; i < NONL_TRAJ_SIZE; i++)
		{
			if ( ( ascAngle[i] < actx->jointAngleDegrees ) &&  ( actx->jointAngleDegrees >= ascAngle[NONL_TRAJ_MAX_INDEX] ) )
			{ // values are increasing, so if we get above it we're done.
				nonLinearKParamx->ascAngleIndex = i;
				i = NONL_TRAJ_SIZE; // end loop
			} else if (actx->jointAngleDegrees <= (ascAngle[NONL_TRAJ_MAX_INDEX] + NONL_TRAJ_ANGLE_END_TOLERANCE) )
			{ // jump into descending mode
				nonLinearKParamx->earlyLateFlag = 1;
				i = NONL_TRAJ_SIZE;
			}
		}
	} else if (nonLinearKParamx->earlyLateFlag == 1)
	{ // Late Stance, powered pushoff going into toe-off,
		for(int16_t i = NONL_TRAJ_MAX_INDEX; i >= 0; i--)
		{
			if ( ( descAngle[i] > actx->jointAngleDegrees   ) &&  ( actx->jointAngleDegrees <= descAngle[0] ) ) // change in here, was: [NONL_TRAJ_MAX_INDEX]
			{ // values are increasing, so if we get above it we're done.
				nonLinearKParamx->descAngleIndex = i;
				i = -1; // end loop
			} else if (actx->jointAngleDegrees >= ascAngle[0] - NONL_TRAJ_ANGLE_END_TOLERANCE )
			{ // end state
				nonLinearKParamx->earlyLateFlag = 0;
				nonLinearKParamx->currentWalkingState = STATE_EARLY_SWING;
			}
		}
	}


	if (nonLinearKParamx->earlyLateFlag == 0)
	{
		//nonLinearKParamx->stiffnessCurrentVal = fabs( ascTorque[nonLinearKParamx->ascAngleIndex] / (ascAngle[NONL_TRAJ_MAX_INDEX] - ascAngle[nonLinearKParamx->ascAngleIndex]) );
		//wParams->ankleGainsNonLinear.thetaDes = ascAngle[0];

		// just output torque value
		nonLinearKParamx->torqueStiff = wParams->userMass * ascTorque[nonLinearKParamx->ascAngleIndex];
	}
	else if (nonLinearKParamx->earlyLateFlag ==1)
	{
		//nonLinearKParamx->stiffnessCurrentVal = fabs( descTorque[nonLinearKParamx->descAngleIndex] / ( descAngle[nonLinearKParamx->descAngleIndex] - descAngle[0] ) );
		//wParams->ankleGainsNonLinear.thetaDes = descAngle[0];
		// just output torque value
		nonLinearKParamx->torqueStiff = wParams->userMass * descTorque[nonLinearKParamx->descAngleIndex];
	}

	//return nonLinearKParamx->stiffnessCurrentVal;
	return nonLinearKParamx->torqueStiff;
}

#endif //BOARD_TYPE_FLEXSEA_MANAGE

#ifdef __cplusplus
}
#endif


