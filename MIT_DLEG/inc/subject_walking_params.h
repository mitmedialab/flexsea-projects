/*
 * subject_walking_params.h
 *
 *  Created on: Oct 6, 2019
 *      Author: matt
 */

#ifndef FLEXSEA_PROJECTS_MIT_DLEG_INC_SUBJECT_WALKING_PARAMS_H_
#define FLEXSEA_PROJECTS_MIT_DLEG_INC_SUBJECT_WALKING_PARAMS_H_

#include "state_variables.h"
#include "global-config.h"
#include "walking_knee_ankle_state_machine.h"


#ifdef SUBJECT_012
	WalkParams subjectAnkleWalkParams =
	{
		.earlyStanceK0 = 6.2,
		.earlyStanceKF = 0.1,
		.earlyStanceDecayConstant = EARLYSTANCE_DECAY_CONSTANT,
		.virtualHardstopEngagementAngle = 0.0,	//user_data_1.w[1] = 0	  [deg]
		.virtualHardstopK				= 4.5,	//user_data_1.w[2] = 350 [Nm/deg] NOTE: Everett liked this high, Others prefer more like 6.0
		.lspEngagementTorque 			= 50.0,	//user_data_1.w[3] = 7400 [Nm]	// What triggers pushoff
		.lstPGDelTics 					= 70.0,	//user_data_1.w[4] = 30			// Delay to ramp up pushoff power

		.ankleGainsEst.k1 				= 0.0,
		.ankleGainsEst.b 		        = 0.18,
		.ankleGainsEst.thetaDes         = 0.0,

		.ankleGainsMst.k1	 	        = 2.5,
		.ankleGainsMst.b 		        = 0.18,
		.ankleGainsMst.thetaDes         = 0.0,

		.ankleGainsLst.k1 		        = 4.5,
		.ankleGainsLst.b 		        = 0.12,
		.ankleGainsLst.thetaDes         = 14.0,

		.ankleGainsEsw.k1		        = 1.5,
		.ankleGainsEsw.b 		        = 0.18,
		.ankleGainsEsw.thetaDes         = -8.0,

		.ankleGainsLsw.k1 		        = 1.5,
		.ankleGainsLsw.b 		        = 0.18,
		.ankleGainsLsw.thetaDes         = -8.0,

		.passiveVirtualHardstopEngagementAngle  = 0.0,
		.passiveVirtualHardstopK 				= 6.5,
		.passiveVirtualHardstopB 				= 0.15

	};
#elif defined(SUBJECT_013)
	WalkParams subjectAnkleWalkParams =
	{
		.earlyStanceK0 = 6.2,
		.earlyStanceKF = 0.1,
		.earlyStanceDecayConstant = EARLYSTANCE_DECAY_CONSTANT,
		.virtualHardstopEngagementAngle = 0.0,	//user_data_1.w[1] = 0	  [deg]
		.virtualHardstopK				= 4.5,	//user_data_1.w[2] = 350 [Nm/deg] NOTE: Everett liked this high, Others prefer more like 6.0
		.lspEngagementTorque 			= 50.0,	//user_data_1.w[3] = 7400 [Nm]	// What triggers pushoff
		.lstPGDelTics 					= 70.0,	//user_data_1.w[4] = 30			// Delay to ramp up pushoff power

		.ankleGainsEst.k1				= 0.0,
		.ankleGainsEst.b 		        = 0.18,
		.ankleGainsEst.thetaDes         = 0.0,

		.ankleGainsMst.k1	 	        = 2.5,
		.ankleGainsMst.b 		        = 0.18,
		.ankleGainsMst.thetaDes         = 0.0,

		.ankleGainsLst.k1 		        = 4.5,
		.ankleGainsLst.b 		        = 0.12,
		.ankleGainsLst.thetaDes         = 14.0,

		.ankleGainsEsw.k1		        = 1.5,
		.ankleGainsEsw.b 		        = 0.18,
		.ankleGainsEsw.thetaDes         = -8.0,

		.ankleGainsLsw.k1 		        = 1.5,
		.ankleGainsLsw.b 		        = 0.18,
		.ankleGainsLsw.thetaDes         = -8.0,

		.passiveVirtualHardstopEngagementAngle  = 0.0,
		.passiveVirtualHardstopK 				= 6.5,
		.passiveVirtualHardstopB 				= 0.15
	};
#endif


#endif /* FLEXSEA_PROJECTS_MIT_DLEG_INC_SUBJECT_WALKING_PARAMS_H_ */
