#ifndef INC_STATE_VARIABLES
#define INC_STATE_VARIABLES

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//****************************************************************************
// Structure(s):
//****************************************************************************

enum {
	STATE_IDLE = 0,
	STATE_INIT = 1,
	STATE_EARLY_SWING = 2,
	STATE_LATE_SWING = 3,
	STATE_EARLY_STANCE = 4,
    STATE_LATE_STANCE = 5,
    STATE_LATE_STANCE_POWER = 6,
    STATE_EMG_STAND_ON_TOE = 7,
    STATE_LSW_EMG = 8
};

typedef struct{
    int8_t current_state;
    uint16_t on_entry_sm_state;
    uint16_t last_sm_state;

} WalkingStateMachine;

//Gain params
#define NUM_STATES				8
#define NUM_IMPEDANCE_TERMS		4

typedef struct{

	float k1;
	float k2;
	float b;
	float thetaDes;

} GainParams;

// Actuator structure to track sensor values, initially built for the TF08 style actuator
typedef struct act_s
{
    float jointAngle;
    float jointAngleDegrees;
    float jointVel;
    float jointVelDegrees;
    float jointAcc;
    float linkageMomentArm;
    float axialForce;
    float jointTorque;
    float tauMeas;          // torque contribution from series spring
    float tauDes;           // FSM des torque - tauMeas
    float lastJointAngle;
    float lastJointTorque;
    float jointTorqueRate;  // Joint torque rate

    int32_t motorVel;		// motor velocity [rad/s]
    int32_t motorAcc;		// motor acceleration [rad/s/s]
    int16_t regTemp;		// regulate temperature
    int16_t motTemp;		// motor temperature
    int32_t motCurr;		// motor current
    int32_t desiredCurrent; // desired current from getMotorCurrent()
    int32_t currentOpLimit; // current throttling limit
    int8_t safetyFlag;		// todo: consider if necessary

} Act_s;


typedef struct walkParams {

	//Early stance params
    float earlyStanceK0;
    float earlyStanceKF;
    float earlyStanceB;
    float earlyStanceDecayConstant;
    float virtualHardstopK;
    float virtualHardstopEngagementAngle;

    int8_t initializedStateMachineVariables;

    //LSP values
    float lspEngagementTorque;
    float samplesInLSP;
	float pff_gain;
	float pff_exponent;
	float lsp_entry_tq;
	float pff_lumped_gain;
	float virtual_hardstop_tq;
	float pff_ramp_tics;
	int16_t transition_id;
	float lstPGDelTics;

	//biom early stance value
	float scaleFactor;

} WalkParams;

typedef struct linearSpline{
	float xi; // x initial
	float xf; // x final
	float yi; // y initial
	float yf; // y final
	float X; // interpolation x coordinate
	float Y; // interpolation y coordinate
	float theta_set_fsm;
	float res_factor; // resolution factor
	uint32_t time_state;
} LinearSpline;

typedef struct cubicSpline{
	float xi; // x initial
	float x_int; // x intermediate
	float xf; // x final
	float yi; // y initial
	float y_int; // y intermediate
	float yf; // y final
	float X; // interpolation x coordinate
	float Y; // interpolation Y coordinate
	float theta_set_fsm;
	float theta_set_fsm_int;
	float theta_set_fsm_end;
	float res_factor; // resolution factor
	uint32_t time_state;
	float a1; // coefficients for the polynomial
	float a2;
	float b1;
	float b2;
} CubicSpline;



//****************************************************************************
// Shared variable(s)
//****************************************************************************
extern int8_t fsm1State;
extern float currentScalar;


#ifdef __cplusplus
}
#endif

#endif //INC_STATE_VARIABLES
