#ifndef __TASKMACHINE_H__
#define __TASKMACHINE_H__

#include "flexsea_user_structs.h"
#include "kinematics_methods.h"
#include "user-mn-MIT-DLeg.h"
#include <stdio.h>
#include <math.h>
#include <float.h>
#include "terrain_state_machine.h"
#include "filtering_methods.h"
#include "heuristic_methods.h"
#include "state_variables.h"



 //Timing constants
#define SAMPLE_RATE_HZ 1000.0
#define SAMPLE_PERIOD_S 1.0/SAMPLE_RATE_HZ

 //Default filter coefficients
#define FILTA 0.95
#define FILTB 0.05
#define FILTC 0.99999
#define FILTD 0.00001

//Other important values
#define PI 3.1415926536
#define N_CLASSES 5

//Specify user
//#define POLETEST
//#define USER_RC
#define USER_JG
//#define USER_RJ
//#define USER_AP
//#define USER_NB
//#define USER_PD


struct taskmachine_s
{

	int control_mode;
	int current_terrain;
	int trial_type;
	int learning_enabled;
	int adaptation_enabled;

	
    float elapsed_samples;
    float latest_foot_off_samples;
    float prev_stride_samples;
    uint8_t in_swing;
    uint8_t passed_min_stance_tq;

    uint8_t gait_event_trigger;  
    uint8_t stride_classified;

    float tq;
    float tq_dot;
    float aa;
    float aa_dot;
    float aa_dot_15hz_filt;
    float tq_prev;
    float aa_prev;
    float mean_swing_tq_nm;


    float net_work_j;
    float power_w;
    float peak_power_w;
    float min_power_w;
    float stance_rom_rad;
    float heelstrike_angle_rad;
    float peak_power_timing_percent;
    float subject_mass_kg;

    float* net_work_error_j_p_kg;
    float* stance_rom_error_rad;
    float* heelstrike_angle_error_rad;
    float* peak_power_timing_error_percent;

    bool net_work_within_bounds;
    bool stance_rom_within_bounds;
    bool heelstrike_angle_within_bounds;
    bool peak_power_timing_within_bounds;

};


struct taskmachine_s* get_task_machine();
void task_machine_demux(struct rigid_s* rigid, Act_s* act);


enum Task_Machine_States {
	INIT_TASK_MACHINE,
	INIT_LEARNING,
	INIT_KINEMATICS,
	INIT_TERRAIN_STATE_MACHINE,
	RUN_TASK_MACHINE,
};

enum Gait_Modes {
	MODE_FLAT = 0,
	MODE_URAMP = 1,
	MODE_DRAMP = 2,
	MODE_USTAIRS = 3,
	MODE_DSTAIRS = 4,
    MODE_NOMINAL = 5,
    MODE_POSITION = 6,
	MODE_HEURISTIC = 7,
};

enum Gait_Events {
    GAIT_EVENT_DEFAULT = 0,
    GAIT_EVENT_FOOT_ON = 1,
    GAIT_EVENT_FOOT_STATIC = 2,
    GAIT_EVENT_FOOT_OFF = 3,
    GAIT_EVENT_WINDOW_CLOSE = 4,
};

 enum Terrain_Classes {
	 K_DEFAULT = -1,
     K_FLAT = 0,
 	K_URAMP = 1,
 	K_DRAMP = 2,
 	K_USTAIRS = 3,
 	K_DSTAIRS = 4,
 };

#endif
