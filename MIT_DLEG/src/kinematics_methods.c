


#include "kinematics_methods.h"



 //Kinematics constants (copied from matlab pil)
#define GRAVITY_MPS2 9.8
#define GRAVITY_SQ GRAVITY_MPS2*GRAVITY_MPS2
#define GYRO_LSB_PER_DPS 32.8  //per http://dephy.com/wiki/flexsea/doku.php?id=units
#define ACCEL_LSB_PER_G 8192.0   //per http://dephy.com/wiki/flexsea/doku.php?id=units
#define ANKLE_POS_IMU_FRAME_X_M 0.0  //Frontal axis (medial->lateral)
#define ANKLE_POS_IMU_FRAME_Y_M -0.00445  //Longitudinal axis (bottom->top)
#define ANKLE_POS_IMU_FRAME_Z_M -0.0605  //Sagittal axis (back->front)
#define ACCEL_MPS2_PER_LSB  GRAVITY_MPS2 / ACCEL_LSB_PER_G
#define N_ACCEL_MPS2_PER_LSB -1.0*ACCEL_MPS2_PER_LSB
#define GYRO_RPS_PER_LSB RAD_PER_DEG / GYRO_LSB_PER_DPS
#define MIN_ACCEL_SUMSQR_MEAN_OFFSET_FOR_RESCALING 1.0
#define MIN_ACCEL_QUIET_SAMPLES_FOR_RESCALING 1000

//Foot static constants
#define MIN_TQ_FOR_FOOT_STATIC 5.0
#define AA_DOT_AOMEGAX_ERROR_THRESH_HIGH 0.4
#define AA_DOT_AOMEGAX_ERROR_THRESH_LOW 0.2
#define LOW_AA_DOT_AOMEGAX_ERROR_COUNT_THRESH 50

static struct kinematics_s kin;

static float ANKLE_TO_IMU_SAGITTAL_PLANE_M;

static void update_ankle_translations(){
	
	if (fabs(kin.pAz) < 1.0){
		float aOmegaXSquared = kin.aOmegaX*kin.aOmegaX;
		float SaAy = kin.aAccY - aOmegaXSquared*ANKLE_POS_IMU_FRAME_Y_M - SAMPLE_RATE_HZ*kin.daOmegaX*ANKLE_POS_IMU_FRAME_Z_M;
		float SaAz = -1.0*kin.aAccZ - aOmegaXSquared*ANKLE_POS_IMU_FRAME_Z_M + SAMPLE_RATE_HZ*kin.daOmegaX*ANKLE_POS_IMU_FRAME_Y_M;

		kin.aAy = -1.0*kin.rot1*SaAy + kin.rot3*SaAz;
		kin.aAz = -1.0*kin.rot3*SaAy - kin.rot1*SaAz - GRAVITY_MPS2;

		kin.vAy = kin.vAy + SAMPLE_PERIOD*kin.aAy;
		kin.vAz = kin.vAz + SAMPLE_PERIOD*kin.aAz;

		kin.pAy = kin.pAy + SAMPLE_PERIOD*kin.vAy;
		kin.pAz = kin.pAz + SAMPLE_PERIOD*kin.vAz;

		float vAySq = kin.vAy*kin.vAy;
		float vAzSq = kin.vAz*kin.vAz;
		kin.sinSqAttackAngle = (((kin.vAz > 0) - (kin.vAz < 0))*vAzSq)/(vAySq+vAzSq);
	}


}

static void reset_kinematics(struct taskmachine_s* tm){
	kin.aa_dot_aOmegaX_error_prev_prev = kin.aa_dot_aOmegaX_error;
	kin.aa_dot_aOmegaX_error_prev = kin.aa_dot_aOmegaX_error;
	kin.aa_dot_aOmegaX_error = FILTA *kin.aa_dot_aOmegaX_error + FILTB*(tm->aa_dot - kin.aOmegaX);

	if (!tm->in_swing && tm->low_torque_counter == 0){
		float abserr = fabs(kin.aa_dot_aOmegaX_error);
		float abserrprev = fabs(kin.aa_dot_aOmegaX_error_prev);
		float abserrprev2 = fabs(kin.aa_dot_aOmegaX_error_prev_prev);
		float abstq = fabs(tm->tq);
		if (abserr < abserrprev &&
		  abserr < AA_DOT_AOMEGAX_ERROR_THRESH_HIGH && abstq > MIN_TQ_FOR_FOOT_STATIC){
			kin.updateOrientation = 1;
		}
		if (tm->tq > tm->tq_prev){
			if (abserr > abserrprev && abserrprev < abserrprev2 &&
					((abserr >= 0 && abserrprev >= 0 && abserrprev2 >= 0) ||
					(abserr < 0 && abserrprev < 0 && abserrprev2 < 0)) ){
				kin.resetPosition = 1;
			}

			if (kin.low_aa_dot_aOmegaX_error_counter > LOW_AA_DOT_AOMEGAX_ERROR_COUNT_THRESH){
				kin.resetPosition = 2;
			}
		}

		if (kin.updateOrientation){
			float zAccWithCentripetalAccCompensation = (kin.aAccZ - kin.aOmegaX*kin.aOmegaX*ANKLE_TO_IMU_SAGITTAL_PLANE_M);
			float yAccWithTangentialAccCompensation = (kin.aAccY + kin.daOmegaX*SAMPLE_RATE_HZ*ANKLE_TO_IMU_SAGITTAL_PLANE_M);
			float accNormReciprocal	= 1.0/sqrtf(yAccWithTangentialAccCompensation*yAccWithTangentialAccCompensation + zAccWithCentripetalAccCompensation*zAccWithCentripetalAccCompensation);
			float costheta = zAccWithCentripetalAccCompensation*accNormReciprocal;
			float sintheta = yAccWithTangentialAccCompensation*accNormReciprocal;
			kin.rot1 = FILTA*kin.rot1 + FILTB*costheta;
			kin.rot3 = FILTA*kin.rot3 - FILTB*sintheta;
		}
		if (kin.resetPosition){
			kin.vAy = 0.0f;
			kin.vAz = 0.0f;
			kin.pAy = 0.0f;
			kin.pAz = 0.0f;
			kin.iaOmegaX = 0.0f;
			kin.iaAccY = 0.0f;
			kin.iaAccZ = 0.0f;
			kin.latest_foot_static_samples = tm->elapsed_samples;
		}

		if (abserr < AA_DOT_AOMEGAX_ERROR_THRESH_LOW){
			kin.low_aa_dot_aOmegaX_error_counter = kin.low_aa_dot_aOmegaX_error_counter + 1;
		}

		if (tm->gait_event_trigger == GAIT_EVENT_FOOT_ON){
			kin.low_aa_dot_aOmegaX_error_counter = 0;
			kin.vAy = 0.0f;
			kin.vAz = 0.0f;
			kin.pAy = 0.0f;
			kin.pAz = 0.0f;
			kin.latest_foot_static_samples = tm->elapsed_samples;
		}

	}
}




static void update_acc( struct fx_rigid_mn_s* mn){
	kin.aAccYprev = kin.aAccZ;
	kin.aAccZprev = kin.aAccZ;
	kin.aAccX = FILTA*kin.aAccX + FILTB * (kin.aAccXYZscaling * (float) mn->accel.x);
	kin.aAccY = FILTA*kin.aAccY  + FILTB * (kin.aAccXYZscaling * (float) mn->accel.z);
	kin.aAccZ = FILTA*kin.aAccZ  + FILTB * (kin.aAccXYZscaling * (float) mn->accel.y);
}

static void update_omega(struct fx_rigid_mn_s* mn){
	kin.aOmegaXprev = kin.aOmegaX;
	kin.aOmegaX = FILTA*kin.aOmegaX + FILTB*(GYRO_RPS_PER_LSB * (float) mn->gyro.x + kin.aOmegaXbias);
	kin.aOmegaY = FILTA*kin.aOmegaY + FILTB*(GYRO_RPS_PER_LSB * (float) mn->gyro.z + kin.aOmegaYbias);
	kin.aOmegaZ = FILTA*kin.aOmegaZ + FILTB*(GYRO_RPS_PER_LSB * (float) mn->gyro.y + kin.aOmegaZbias);
}

static void correct_gyro_bias(){
	kin.aOmegaXbias = FILTC*kin.aOmegaXbias - FILTD*kin.aOmegaX;
	kin.aOmegaYbias = FILTC*kin.aOmegaYbias - FILTD*kin.aOmegaY;
	kin.aOmegaZbias = FILTC*kin.aOmegaZbias - FILTD*kin.aOmegaZ;
}

static void correct_accel_scaling(){
	kin.accelSumSqr = kin.aAccX*kin.aAccX + kin.aAccY*kin.aAccY + kin.aAccZ*kin.aAccZ;
	kin.meanAccelSumSqr = FILTA*kin.meanAccelSumSqr + FILTB*kin.accelSumSqr;

	if (fabs(kin.meanAccelSumSqr - kin.accelSumSqr) < MIN_ACCEL_SUMSQR_MEAN_OFFSET_FOR_RESCALING){
		kin.accelQuietSamples = kin.accelQuietSamples + 1;
	}else{
		kin.accelQuietSamples = 0;
	}

	if (kin.accelQuietSamples > MIN_ACCEL_QUIET_SAMPLES_FOR_RESCALING){
		kin.aAccXYZscaling = kin.aAccXYZscaling * sqrtf(GRAVITY_SQ/kin.meanAccelSumSqr);
		kin.accelQuietSamples = 0;
	}
}

static void update_integrals_and_derivatives(){
	kin.iaAccY = kin.iaAccY + kin.aAccY;
	kin.daAccY = FILTA*kin.daAccY + FILTB*(kin.aAccY - kin.aAccYprev);
	kin.iaAccZ = kin.iaAccZ + kin.aAccZ;
	kin.daAccZ = FILTA*kin.daAccZ + FILTB*(kin.aAccZ - kin.aAccZprev);
	kin.iaOmegaX = kin.iaOmegaX - kin.aOmegaX;
	kin.daOmegaX = FILTA*kin.daOmegaX + FILTB*(kin.aOmegaX - kin.aOmegaXprev);
}


static void update_rotation_matrix(){
	float rotprev1 = kin.rot1;

	kin.rot1 = kin.rot1 - kin.rot3*kin.aOmegaX * SAMPLE_PERIOD;
	kin.rot3 = kin.rot3 + rotprev1*kin.aOmegaX * SAMPLE_PERIOD;

//	float rotnormReciprocal = 1.0/sqrtf(kin.rot1*kin.rot1 + kin.rot3*kin.rot3);
//	kin.rot1 = kin.rot1*rotnormReciprocal;
//	kin.rot3 = kin.rot3*rotnormReciprocal;
}

void update_kinematics(struct fx_rigid_mn_s* mn, struct taskmachine_s* tm){

	kin.updateOrientation = 0;
	kin.resetPosition = 0;

	update_acc(mn);
	update_omega(mn);
	correct_gyro_bias();
	correct_accel_scaling();

	update_integrals_and_derivatives();
	update_rotation_matrix();
	update_ankle_translations();

	reset_kinematics(tm);

}


void init_kinematics(){

	ANKLE_TO_IMU_SAGITTAL_PLANE_M = sqrtf(ANKLE_POS_IMU_FRAME_Y_M*ANKLE_POS_IMU_FRAME_Y_M + ANKLE_POS_IMU_FRAME_Z_M*ANKLE_POS_IMU_FRAME_Z_M);

	kin.aOmegaX = 0.0;
    kin.aOmegaY = 0.0;
    kin.aOmegaZ = 0.0;
    kin.aAccX = 0.0;
    kin.aAccY = 0.0;
    kin.aAccZ = 0.0;
    kin.iaAccY = 0.0;
    kin.daAccY = 0.0;
    kin.iaAccZ = 0.0;
    kin.daAccZ = 0.0;
    kin.iaOmegaX = 0.0;
    kin.daOmegaX = 0.0;
    kin.aAccYprev = 0.0;
    kin.aAccZprev = 0.0;
    kin.aOmegaXprev = 0.0;
    kin.rot1 = 0.0;
    kin.rot3 = 0.0;
    kin.sinSqAttackAngle =  0;

    kin.aOmegaXbias = 0.0;
    kin.aOmegaYbias = 0.0;
    kin.aOmegaZbias = 0.0;

    kin.aAy = 0.0;
    kin.aAz = 0.0;
    kin.vAy = 0.0;
    kin.vAz = 0.0;
    kin.pAy = 0.0;
    kin.pAz = 0.0;

    kin.aAccXYZscaling = ACCEL_MPS2_PER_LSB;
    kin.accelQuietSamples = 0;
    kin.meanAccelSumSqr = 0.0;
    kin.accelSumSqr = 0.0;

    kin.updateOrientation = 0;
    kin.resetPosition = 0;

    kin.aa_dot_aOmegaX_error = 0.0;
    kin.aa_dot_aOmegaX_error_prev = 0.0;
    kin.aa_dot_aOmegaX_error_prev_prev = 0.0;
    kin.latest_foot_static_samples = 0.0;
    kin.low_aa_dot_aOmegaX_error_counter = 0;
}

struct kinematics_s* get_kinematics(){
	return &kin;
}


