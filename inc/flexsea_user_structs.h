/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-system' System commands & functions
	Copyright (C) 2016 Dephy, Inc. <http://dephy.com/>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] flexsea_global_structs: contains all the data structures
	used across the project
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-12-08 | jfduval | Initial release
	*
****************************************************************************/

#ifndef INC_FLEXSEA_USER_STRUCT_H
#define INC_FLEXSEA_USER_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include <stdint.h>
#include <flexsea_global_structs.h>

//****************************************************************************
// Definition(s):
//****************************************************************************

//****************************************************************************
// Structure(s):
//****************************************************************************

struct motortb_s
{
	int16_t ex1[6];
	int16_t ex2[6];
	int16_t mn1[4];
};

struct filtvar_s
{
	int64_t raws[2];
	int64_t filts[2];
	int32_t raw;
	int32_t filt;
};

//FlexSEA-Rigid:

struct fx_rigid_re_s
{
	uint16_t vb;
	uint16_t vg;
	uint16_t v5;
	int16_t current;
	int8_t temp;
	uint8_t button;
	uint8_t state;

	uint16_t status;
};

struct decoded_fx_rigid_mn_s
{
	struct decoded_xyz_s gyro;
	struct decoded_xyz_s accel;
	struct decoded_xyz_s magneto;
};

//Assumes uniform priors at the moment
struct classifier_s
{
	float* A;
	float* B;
	float* score_k;
	int k_pred;
};



//Assumes uniform priors at the moment
struct learner_s
{
	float* mu_k;
	float* mu_prev;
	float* mu;
	float* sigma;

	float* feats_prev;
	float* pop_k;
	float pop;
	
	int k_est;

	//Intermediate matrix holders
    float* T;
    float* A;
    float* x;
    float* y;
};

struct taskmachine_s
{
	struct classifier_s lda;
	struct learner_s lrn;
	int nFeats;
	float* feats;
};


struct fx_rigid_mn_s
{
	struct xyz_s gyro;
	struct xyz_s accel;
	struct xyz_s magneto; 	//Useless

	float aOmegaX;
	float aOmegaY;
	float aOmegaZ;
	float aAccX;
	float aAccY;
	float aAccZ;

	uint16_t analog[4];
	uint16_t status;

	int16_t genVar[20];

	//Biomech added:
	int16_t mot_temp;
	int32_t userVar[20];

	//Decoded:
	struct decoded_fx_rigid_mn_s decoded;

	//Classifier:
	struct learner_s learner;
	


};

struct fx_rigid_ex_s
{
	uint16_t strain;
	int32_t mot_current;
	int32_t mot_volt;
	int32_t* enc_ang;
	int32_t* enc_ang_vel;
	int16_t* joint_ang;
	int16_t* joint_ang_vel;
	int16_t* joint_ang_from_mot;
	int32_t mot_acc;

	uint16_t status;

	struct ctrl_s ctrl;

	// --- all those pointer members need to be switched to actual members...
	int32_t _enc_ang_;
	int32_t _enc_ang_vel_;

	int16_t _joint_ang_;
	int16_t _joint_ang_vel_;
	int16_t _joint_ang_from_mot_;
};

struct fx_rigid_ctrl_s
{
	uint32_t timestamp;
	int8_t walkingState;
	int8_t gaitState;

	int16_t* ank_ang_deg;
	int16_t* ank_vel;
	int16_t* ank_ang_from_mot;

	int16_t _ank_ang_deg_;
	int16_t _ank_vel_;
	int16_t _ank_ang_from_mot_;

	int16_t contra_hs;
	int16_t step_energy;
};

struct rigid_s
{
	struct fx_rigid_re_s re;
	struct fx_rigid_mn_s mn;
	struct fx_rigid_ex_s ex;
	struct fx_rigid_ctrl_s	ctrl;
	uint8_t lastOffsetDecoded;
};

struct pocket_s
{
	struct fx_rigid_re_s re;
	struct fx_rigid_mn_s mn;
	struct fx_rigid_ex_s ex[2];
	struct fx_rigid_ctrl_s	ctrl;
	uint8_t lastOffsetDecoded;
};

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern struct taskmachine_s tm;
extern struct motortb_s motortb;
extern int16_t globvar[10];
extern struct rigid_s rigid1, rigid2;
extern struct pocket_s pocket1;
extern int16_t globvar[10];


//****************************************************************************
// Prototype(s):
//****************************************************************************

void initializeUserStructs(void);

#ifdef __cplusplus
}
#endif

#endif	//INC_FLEXSEA_GLOBAL_STRUCT_H
