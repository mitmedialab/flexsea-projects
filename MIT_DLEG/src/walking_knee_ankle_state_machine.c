
#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************
#include "walking_knee_ankle_state_machine.h"
#include <user-mn-MIT-DLeg.h>
//#include "user-mn-MIT-EMG.h"
#include "spline_functions.h"
#include "free_ankle_EMG.h"

//****************************************************************************
// Definition(s):
//****************************************************************************
WalkingStateMachine kneeAnkleStateMachine;
WalkParams ankleWalkParams, kneeWalkParams;
CubicSpline cubicSpline;

//NOTE: All of the damping values have been reduced by 1/10 due to controller
// Gain Parameters are modified to match our joint angle convention (RHR for right ankle, wearer's perspective). Positive Plantaflexion
GainParams ankleGainsEst = {1.5, 0.0, 0.1, -10.0};	// may want to increase this damping, at least.
GainParams ankleGainsMst = {1.5, 0.0, 0.10, 0.0};	// may want to increase this damping, at least.
GainParams ankleGainsLst = {4.0, 0.0, 0.03, 14.0};
GainParams ankleGainsEsw = {1.5, 0.0, 0.1, -10.0};
GainParams ankleGainsLsw = {1.5, 0.0,  0.1, -10.0};
GainParams ankleGainsEMG = {0.0, 0.0, 0.0, 0.0};

float splineTime = 100.0;

//Knee, Positive Knee Flexion
GainParams kneeGainsEst = {2.5, 0.0, 0.02, 10.0};
GainParams kneeGainsMst = {3.5, 0.0, 0.02, 10.0};
GainParams kneeGainsLst = {2.5, 0.0, 0.02, 15.0};
GainParams kneeGainsEsw = {1.5, 0.0, 0.02, 50.0};
GainParams kneeGainsLsw = {1.5, 0.0, 0.02, 50.0};




#ifdef BOARD_TYPE_FLEXSEA_MANAGE

//****************************************************************************
// Functions:
//****************************************************************************

/** Impedance Control Level-ground Walking FSM
	Ankle is SLAVE:	determines walking state. Sends out the state to the Knee
	Knee is MASTER: receives kneeAnkleStateMachine.currentState updates from
					the Ankle actuator, and changes impedance settings accordingly
	Param: actx(Act_s) - Actuator structure to track sensor values
	Param: actx(Act_s)

	ptorqueDes pointer to float meant to be updated with desired torque TODO:find out what this is
*/ //void setKneeAnkleFlatGroundFSM(Act_s *actx);
//void setKneeAnkleFlatGroundFSM(Act_s *actx, Act_s *actx) {
void setKneeAnkleFlatGroundFSM(Act_s *actx) {

    static int8_t isTransitioning = 0;
    static uint32_t timeInState = 0;
    static int8_t passedStanceThresh = 0;

    ankleGainsMst = ankleGainsEst;
    ankleGainsEsw = ankleGainsLsw;

    kneeAnkleStateMachine.onEntrySlaveSmState = kneeAnkleStateMachine.slaveCurrentState; // save the state on entry, assigned to last_currentState on exit

    //TODO: See if this is reasonable, change state to slave state, unless in early swing. In that case only switch when local state decides to change
//    if (kneeAnkleStateMachine.currentState != STATE_EARLY_SWING)
//    {
//    	kneeAnkleStateMachine.currentState = kneeAnkleStateMachine.slaveCurrentState;
//    }

    kneeAnkleStateMachine.onEntrySmState = kneeAnkleStateMachine.currentState; // save the state on entry, assigned to last_currentState on exit

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

        case STATE_IDLE:
        {
        	//error handling here (should never be in STATE_IDLE by the time you get here)
            break;
        }
        case STATE_INIT:
        {
        	#ifdef IS_ANKLE
        		actx->tauDes = 0.0;	// Initialize to no commanded torque
			#elif defined(IS_KNEE)
        		actx->tauDes = 0.0;
			#endif

        	kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;	// enter into early stance, this has stability.

            break;
        }
        case STATE_EARLY_STANCE:
        {
			if (isTransitioning && !passedStanceThresh) {
				ankleWalkParams.scaleFactor = 1.0;
//					ankleGainsEst.k1 = ankleWalkParams.earlyStanceK0;
//				ankleGainsEst.thetaDes = actx->jointAngleDegrees;	// used by updateImpedanceParams, if in use, could be turned off.
	//			updateImpedanceParams(actx, &ankleWalkParams);	//Todo: Probably want to bring this back to ease into stance, though Hugh prefers a stiff ankle - why it was removed
				passedStanceThresh = 0;

				#ifdef USE_EMG
				resetVirtualJoint(actx->jointAngleDegrees,0,0);
				#endif //USE_EMG
			}

			#ifdef IS_ANKLE
				updateAnkleVirtualHardstopTorque(actx, &ankleWalkParams);


				#ifdef USE_EMG
				updateVirtualJoint(&ankleGainsEMG);
  	  	  	  	#endif //USE_EMG


				actx->tauDes = ankleWalkParams.virtualHardstopTq + getImpedanceTorque(actx, ankleGainsEst.k1, ankleGainsEst.b, ankleGainsEst.thetaDes) + getImpedanceTorque(actx, ankleGainsEMG.k1, ankleGainsEMG.b, ankleGainsEMG.thetaDes);

				if (JNT_ORIENT*actx->jointAngleDegrees > ankleWalkParams.virtualHardstopEngagementAngle) {
					kneeAnkleStateMachine.currentState = STATE_MID_STANCE;
					passedStanceThresh = 1;

				}
			#elif defined(IS_KNEE)
				actx->tauDes = getImpedanceTorque(actx, kneeGainsEst.k1, kneeGainsEst.b, kneeGainsEst.thetaDes);
			#endif

			break;
        }
        case STATE_MID_STANCE:
        {
			if (isTransitioning) {

			}

			#ifdef IS_ANKLE
        		updateAnkleVirtualHardstopTorque(actx, &ankleWalkParams);	// Bring in
        		actx->tauDes = ankleWalkParams.virtualHardstopTq + getImpedanceTorque(actx, ankleGainsMst.k1, ankleGainsMst.b, ankleGainsMst.thetaDes);

    			// Stance transition vectors, only go into next state. This is a stable place to be.

    			if (actx->jointTorque > ankleWalkParams.lspEngagementTorque) {
    				kneeAnkleStateMachine.currentState = STATE_LATE_STANCE_POWER;      //Transition occurs even if the early swing motion is not finished
    			}

			#elif defined(IS_KNEE)
        		actx->tauDes = getImpedanceTorque(actx, kneeGainsMst.k1, kneeGainsMst.b, kneeGainsMst.thetaDes);
			#endif

        	break;
        }
        case STATE_LATE_STANCE_POWER: //2
        {
			if (isTransitioning) {
				ankleWalkParams.samplesInLSP = 0.0;
				ankleWalkParams.lspEntryTq = actx->jointTorque;
			}

			#ifdef IS_ANKLE
				// This is the scaling factor for ramping into powered pushoff
				if (ankleWalkParams.samplesInLSP < ankleWalkParams.lstPGDelTics){
					ankleWalkParams.samplesInLSP++;
				}

				updateAnkleVirtualHardstopTorque(actx, &ankleWalkParams);
				//Linear ramp to push off
				actx->tauDes = ankleWalkParams.virtualHardstopTq + (ankleWalkParams.samplesInLSP/ankleWalkParams.lstPGDelTics) * getImpedanceTorque(actx, ankleGainsLst.k1, ankleGainsLst.b, ankleGainsLst.thetaDes);

				//Late Stance Power transition vectors
					//todo: Should there be a way to jump back into early_stance in the event of running?
				if (abs(actx->jointTorque) < ANKLE_UNLOADED_TORQUE_THRESH && timeInState > LST_TO_ESW_DELAY && (actx->jointAngleDegrees >=  ankleGainsLst.thetaDes -1.0) ) {	// not sure we need the timeInState? what's the point? just maker sure it's kicking?
					kneeAnkleStateMachine.currentState = STATE_EARLY_SWING;
				}
			#elif defined(IS_KNEE)
				actx->tauDes = getImpedanceTorque(actx, kneeGainsLst.k1, kneeGainsLst.b, kneeGainsLst.thetaDes);
			#endif

            break;
        }
        case STATE_EARLY_SWING:
        {
			//Put anything you want to run ONCE during state entry.
			if (isTransitioning) {
				ankleWalkParams.virtualHardstopTq = 0.0;

				// initialize cubic spline params once
//				initializeCubicSplineParams(&cubicSpline, actx, ankleGainsEsw, splineTime); // last parameter is res_factor (delta X - time)
			}

			#ifdef IS_ANKLE
				// Cubic Spline NOT WORKING; UNTESTED
//				calcCubicSpline(&cubicSpline);
//				ankleGainsEsw.thetaDes = cubicSpline.Y; //new thetaDes after cubic spline

				actx->tauDes = getImpedanceTorque(actx, ankleGainsEsw.k1, ankleGainsEsw.b, ankleGainsEsw.thetaDes);

				if(actx->jointAngleDegrees <= ankleGainsEsw.thetaDes || timeInState >= ESW_TO_LSW_DELAY)
//				if(actx->jointAngleDegrees <= ankleGainsEsw.thetaDes)
				{
					kneeAnkleStateMachine.currentState = STATE_LATE_SWING;
				}

			#elif defined(IS_KNEE)
				actx->tauDes = getImpedanceTorque(actx, kneeGainsEsw.k1, kneeGainsEsw.b, kneeGainsEsw.thetaDes);

				// This State is different, Knee decides on its own when to transition to STATE_LATE_SWING
				if(actx->jointVelDegrees < 0 )
				{
					kneeAnkleStateMachine.currentState = STATE_LATE_SWING;
				}

			#endif

			//Early Swing transition vectors
//			if (timeInState >= 200) {
//				kneeAnkleStateMachine.currentState = STATE_LATE_SWING;      //Transition occurs even if the early swing motion is not finished
//			}
			//Transition occurs when toe swings up or time elapsed

			break; // case STATE_EARLY_SWING
        }
		case STATE_LATE_SWING:
		{
			if (isTransitioning) {
				ankleWalkParams.transitionId = 0;
			}

			#ifdef IS_ANKLE

				actx->tauDes = getImpedanceTorque(actx, ankleGainsEsw.k1, ankleGainsEsw.b, ankleGainsEsw.thetaDes);

				//---------------------- LATE SWING TRANSITION VECTORS ----------------------//
				if(timeInState > LSW_TO_EST_DELAY) {

					// Late Swing -> Early Stance (hard toe strike) - Running
					if ( actx->jointTorque > HARD_TOESTRIKE_TORQUE_THRESH || actx->jointVelDegrees > HARD_TOESTRIKE_VEL_THRESH_DEG)
					{
//						kneeAnkleStateMachine.currentState = STATE_MID_STANCE;
						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
						ankleWalkParams.transitionId = 0;
					}
					// Late Swing -> Early Stance (hard heal strike)
					else if ( fabs(actx->jointTorque) > HARD_HEELSTRIKE_TORQUE_THRESH && fabs(actx->jointTorqueRate) > GENTLE_HEELSTRIKE_TORQ_RATE_THRESH)
					{
						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
						ankleWalkParams.transitionId = 1;
					}
					// Late Swing -> Early Stance (gentle heal strike) - Condition 2 -
					else if ( actx->jointTorque < GENTLE_HEELSTRIKE_TORQUE_THRESH && actx->jointAngleDegrees <= (ankleGainsEst.thetaDes + 1.0) )
					{
						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
						ankleWalkParams.transitionId = 2;
					}
					else if ( fabs(actx->jointTorque) > GENTLE_HEELSTRIKE_TORQUE_THRESH)
					{
						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
						ankleWalkParams.transitionId = 3;
					}
					//TODO: might want to go into an idle state of some sort, check how this works.

				}
				//------------------------- END OF TRANSITION VECTORS ------------------------//
			#elif defined(IS_KNEE)
				actx->tauDes = getImpedanceTorque(actx, kneeGainsLsw.k1, kneeGainsLsw.b, kneeGainsLsw.thetaDes);
				if(timeInState > LSW_TO_EST_DELAY)
				{
					if (actx->jointAngleDegrees <= kneeGainsLsw.thetaDes + 1.0)
					{
						kneeAnkleStateMachine.currentState = STATE_EARLY_STANCE;
					}
				}
			#endif

			break;
		}
        default:
        {
            //turn off control.
            actx->tauDes = 0;
            kneeAnkleStateMachine.currentState = STATE_INIT;

            break;
        }
    }

    //update last state in preparation for next loop
    kneeAnkleStateMachine.lastSmState = kneeAnkleStateMachine.onEntrySmState;



}

///** Impedance Control Torque
//	Calculates desired torque based on impedance gain parameters
//
//	Param: gainParams(GainParams) - struct with all the state's impedance parameters
//	Param: actx(Act_s) - Actuator structure to track sensor values
//	Param: wParams(WalkParams) - Parameters relating to walking states
//    Return: float desired torque at joint in NEWTON-METERS
//*/
//float calcJointTorque(GainParams gainParams, Act_s *actx, WalkParams *wParams) {
//
//	return gainParams.k1 * (gainParams.thetaDes - actx->jointAngleDegrees) \
//         - gainParams.b * actx.jointVelDegrees ;
//}



//
/** This ramps down the Stiffness of early stance K, picking up the user and bringing them up to ankleGainsEst.thetaDes
    NOTE/TODO: Hugh may prefer much more stiffness here, this ramps down teh stiffness.
	Param: actx(Act_s) - Actuator structure to track sensor values
	Param: wParams(WalkParams) - Parameters relating to walking states
*/
void updateImpedanceParams(Act_s *actx, WalkParams *wParams) {

	wParams->scaleFactor = wParams->scaleFactor * wParams->earlyStanceDecayConstant;

    ankleGainsEst.k1 = wParams->earlyStanceKF + wParams->scaleFactor * (wParams->earlyStanceK0 - wParams->earlyStanceKF);

    if (actx->jointAngleDegrees < ankleGainsEst.thetaDes) {
    	ankleGainsEst.thetaDes = actx->jointAngleDegrees;
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



#endif //BOARD_TYPE_FLEXSEA_MANAGE

#ifdef __cplusplus
}
#endif

