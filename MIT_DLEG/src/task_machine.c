 

#include "task_machine.h"

// Gait event thresholds
#define EXPECTED_SWING_TQ_NM 0.0
#define TRANSITION_TQ_THRESH_NM 7.0
#define MIN_STANCE_TQ_NM EXPECTED_SWING_TQ_NM + TRANSITION_TQ_THRESH_NM + 5.0
#define TRANSITION_POWER_THRESH_W -0.3
#define PREDICTION_CUTOFF_SAMPLES 100
#define MAX_SWING_TQ_DOT_NM_HZ 100
#define MIN_GAIT_PHASE_SAMPLES 250


static struct taskmachine_s tm;
static int task_machine_demux_state = INIT_TASK_MACHINE;
#if defined(NO_DEVICE)
static float tqraw;
static float aaraw;
float example_stride_aa[] = {0.15, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.19, 0.19, 0.51, 0.51, 0.51, 0.51, 0.72, 0.72, 0.72, 0.72, 0.84, 0.84, 0.84, 0.84, 0.84, 0.84, 0.84, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.83, 0.83, 0.83, 0.77, 0.77, 0.77, 0.77, 0.73, 0.66, 0.66, 0.66, 0.58, 0.48, 0.48, 0.48, 0.48, 0.48, 0.48, 0.48, 0.48, 0.46, 0.50, 0.50, 0.50, 0.50, 0.50, 0.55, 0.61, 0.61, 0.61, 0.61, 0.61, 0.61, 0.61, 0.61, 0.61, 0.68, 0.68, 0.68, 0.68, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.95, 0.95, 0.95, 0.95, 0.84, 0.62, 0.62, 0.62, 0.62, 0.63, 0.63, 0.63, 0.63, 0.63, 0.63, 0.60, 0.60, 0.60, 0.60, 0.53, 0.53, 0.53, 0.53, 0.43, 0.31, 0.31, 0.31, 0.31, 0.31, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.06, 0.06, 0.06, 0.06, 0.06, -0.09, -0.15, -0.15, -0.15, -0.15, -0.15, -0.15, -0.20, -0.39, -0.39, -0.39, -0.46, -0.46, -0.52, -0.52, -0.52, -0.52, -0.52, -0.52, -0.68, -0.68, -0.68, -0.75, -0.75, -0.75, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.92, -0.92, -1.00, -1.00, -1.00, -1.00, -1.00, -1.00, -1.00, -1.00, -1.00, -1.19, -1.19, -1.19, -1.27, -1.37, -1.37, -1.65, -1.65, -1.65, -1.65, -1.75, -1.75, -1.75, -1.75, -1.75, -1.75, -1.75, -1.75, -1.94, -2.04, -2.04, -2.04, -2.13, -2.13, -2.13, -2.13, -2.13, -2.13, -2.13, -2.22, -2.22, -2.22, -2.22, -2.22, -2.30, -2.30, -2.30, -2.30, -2.30, -2.30, -2.30, -2.37, -2.37, -2.50, -2.50, -2.50, -2.50, -2.61, -2.61, -2.68, -2.70, -2.70, -2.70, -2.72, -2.72, -2.72, -2.72, -2.72, -2.72, -2.72, -2.72, -2.73, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.73, -2.73, -2.73, -2.73, -2.73, -2.73, -2.73, -2.71, -2.71, -2.71, -2.71, -2.71, -2.66, -2.66, -2.66, -2.66, -2.63, -2.59, -2.56, -2.56, -2.56, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.51, -2.51, -2.51, -2.51, -2.51, -2.51, -2.46, -2.46, -2.46, -2.46, -2.46, -2.46, -2.46, -2.46, -2.45, -2.45, -2.45, -2.45, -2.44, -2.44, -2.44, -2.44, -2.44, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.40, -2.40, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.43, -2.43, -2.43, -2.46, -2.46, -2.46, -2.48, -2.48, -2.48, -2.51, -2.51, -2.51, -2.51, -2.51, -2.51, -2.51, -2.51, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.60, -2.60, -2.60, -2.60, -2.60, -2.60, -2.64, -2.64, -2.75, -2.75, -2.79, -2.79, -2.79, -2.83, -2.83, -2.83, -2.89, -2.89, -2.89, -2.89, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -3.01, -3.01, -3.01, -3.04, -3.04, -3.04, -3.04, -3.04, -3.04, -3.10, -3.10, -3.10, -3.10, -3.10, -3.10, -3.17, -3.17, -3.29, -3.29, -3.29, -3.29, -3.35, -3.39, -3.39, -3.39, -3.39, -3.39, -3.39, -3.46, -3.46, -3.46, -3.46, -3.46, -3.51, -3.56, -3.56, -3.56, -3.56, -3.62, -3.62, -3.62, -3.62, -3.62, -3.67, -3.67, -3.67, -3.67, -3.67, -3.67, -3.67, -3.67, -3.67, -3.67, -3.71, -3.77, -3.77, -3.77, -3.77, -3.77, -3.83, -3.83, -3.88, -4.03, -4.03, -4.08, -4.08, -4.08, -4.08, -4.12, -4.12, -4.12, -4.12, -4.23, -4.23, -4.23, -4.23, -4.23, -4.23, -4.23, -4.23, -4.23, -4.23, -4.28, -4.28, -4.28, -4.32, -4.32, -4.38, -4.38, -4.38, -4.38, -4.43, -4.43, -4.43, -4.43, -4.43, -4.43, -4.43, -4.54, -4.58, -4.58, -4.58, -4.58, -4.58, -4.58, -4.66, -4.82, -4.82, -4.82, -4.82, -4.87, -4.87, -4.87, -4.87, -4.87, -4.87, -4.87, -4.99, -4.99, -4.99, -4.99, -4.99, -5.05, -5.05, -5.11, -5.11, -5.11, -5.16, -5.16, -5.16, -5.16, -5.16, -5.16, -5.16, -5.16, -5.21, -5.21, -5.21, -5.21, -5.21, -5.21, -5.21, -5.26, -5.26, -5.26, -5.26, -5.26, -5.26, -5.26, -5.26, -5.43, -5.43, -5.59, -5.59, -5.66, -5.66, -5.66, -5.66, -5.66, -5.79, -5.79, -5.79, -5.79, -5.79, -5.79, -5.85, -5.92, -5.92, -5.92, -5.92, -5.99, -5.99, -5.99, -5.99, -5.99, -5.99, -6.06, -6.06, -6.06, -6.06, -6.12, -6.12, -6.12, -6.12, -6.12, -6.27, -6.27, -6.27, -6.27, -6.27, -6.27, -6.33, -6.41, -6.54, -6.54, -6.54, -6.54, -6.61, -6.61, -6.69, -6.69, -6.69, -6.76, -6.76, -6.84, -6.91, -6.91, -6.91, -6.91, -6.98, -6.98, -7.06, -7.06, -7.06, -7.06, -7.06, -7.06, -7.06, -7.12, -7.12, -7.12, -7.12, -7.19, -7.19, -7.19, -7.19, -7.19, -7.19, -7.19, -7.19, -7.34, -7.34, -7.34, -7.34, -7.34, -7.50, -7.50, -7.50, -7.69, -7.69, -7.76, -7.76, -7.76, -7.76, -7.83, -7.83, -7.83, -7.83, -7.83, -7.97, -7.97, -7.97, -7.97, -7.97, -7.97, -7.97, -8.04, -8.04, -8.10, -8.10, -8.17, -8.17, -8.17, -8.17, -8.17, -8.17, -8.17, -8.23, -8.23, -8.23, -8.23, -8.23, -8.30, -8.30, -8.30, -8.30, -8.45, -8.45, -8.45, -8.45, -8.45, -8.45, -8.59, -8.59, -8.83, -8.83, -8.83, -8.91, -8.91, -8.91, -8.91, -8.91, -8.91, -8.91, -8.99, -9.13, -9.13, -9.13, -9.13, -9.13, -9.13, -9.20, -9.27, -9.27, -9.27, -9.27, -9.27, -9.27, -9.27, -9.27, -9.27, -9.27, -9.34, -9.34, -9.43, -9.43, -9.43, -9.43, -9.43, -9.43, -9.43, -9.43, -9.50, -9.50, -9.50, -9.50, -9.50, -9.75, -9.75, -9.98, -9.98, -9.98, -10.05, -10.05, -10.05, -10.05, -10.14, -10.14, -10.14, -10.22, -10.22, -10.31, -10.31, -10.31, -10.31, -10.39, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.57, -10.57, -10.65, -10.65, -10.65, -10.65, -10.65, -10.82, -11.32, -11.32, -11.32, -11.32, -11.40, -11.48, -11.48, -11.48, -11.71, -11.71, -11.79, -11.79, -11.79, -11.79, -11.79, -11.79, -11.79, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.95, -11.95, -11.95, -11.95, -11.95, -11.95, -11.95, -12.10, -12.10, -12.10, -12.10, -12.10, -12.10, -12.23, -12.23, -12.23, -12.23, -12.23, -12.23, -12.23, -12.23, -12.23, -12.23, -12.30, -12.30, -12.36, -12.36, -12.36, -12.36, -12.36, -12.36, -12.36, -12.36, -12.36, -12.36, -12.42, -12.46, -12.46, -12.46, -12.46, -12.46, -12.46, -12.53, -12.53, -12.59, -12.59, -12.59, -12.59, -12.61, -12.61, -12.61, -12.63, -12.63, -12.63, -12.63, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.68, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.65, -12.65, -12.65, -12.60, -12.60, -12.60, -12.60, -12.60, -12.52, -12.52, -12.52, -12.52, -12.52, -12.46, -12.38, -12.13, -12.13, -12.13, -12.13, -12.13, -12.03, -11.92, -11.92, -11.92, -11.92, -11.92, -11.69, -11.69, -11.69, -11.56, -11.56, -11.56, -11.56, -11.42, -11.42, -11.28, -11.28, -11.28, -11.28, -11.28, -11.28, -11.28, -11.28, -11.28, -11.28, -11.13, -11.13, -11.13, -10.97, -10.97, -10.97, -10.97, -10.97, -10.65, -10.65, -10.65, -10.46, -10.46, -10.46, -10.46, -10.26, -10.26, -9.65, -9.65, -9.65, -9.43, -9.43, -9.43, -9.20, -9.20, -9.20, -9.20, -9.20, -9.20, -8.96, -8.96, -8.96, -8.96, -8.96, -8.96, -8.46, -8.46, -8.46, -8.19, -8.19, -8.19, -8.19, -8.19, -8.19, -8.19, -8.19, -8.19, -7.92, -7.92, -7.92, -7.92, -7.92, -7.92, -7.64, -7.64, -7.64, -7.64, -7.64, -7.64, -6.69, -6.69, -5.68, -5.68, -5.39, -5.39, -5.39, -5.39, -5.39, -5.15, -5.15, -5.15, -5.15, -5.15, -5.15, -5.15, -5.15, -4.66, -4.13, -4.13, -4.13, -4.13, -4.13, -4.13, -4.13, -4.13, -4.13, -4.13, -3.85, -3.85, -3.85, -3.85, -3.59, -3.59, -3.59, -3.59, -3.59, -3.59, -3.33, -3.33, -3.33, -3.33, -3.33, -3.06, -3.06, -2.03, -1.79, -1.79, -1.79, -1.79, -1.55, -1.55, -1.55, -1.55, -1.55, -1.55, -1.55, -1.55, -1.55, -0.93, -0.76, -0.76, -0.76, -0.76, -0.76, -0.76, -0.76, -0.76, -0.76, -0.76, -0.63, -0.63, -0.63, -0.63, -0.49, -0.49, -0.49, -0.49, -0.49, -0.49, -0.49, -0.49, -0.49, -0.11, -0.11, 0.13, 0.13, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.23, 0.23, 0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.35, 0.35, 0.35, 0.35, 0.34, 0.34, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.30, 0.30, 0.30, 0.30, 0.30, 0.30, 0.30, 0.30, 0.30, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.25, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.22, 0.22, 0.22, 0.23, 0.23, 0.23, 0.23, 0.23, 0.23, 0.23, 0.23, 0.23, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.19, 0.19, 0.19, 0.19, 0.19, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.21, 0.21, 0.21, 0.21, 0.21, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.17, 0.17, 0.17, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.16, 0.16, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17};
#endif


// input / output arrays for butterworth filtering
static float tq_dot_outputs[] = {0,0,0};
static float tq_dot_inputs[] = {0,0,0};
static float aa_dot_outputs[] = {0,0,0};
static float aa_dot_inputs[] = {0,0,0};

// initializing task machine used to store info on current walking state,
// terrain info, and important metrics
static void init_task_machine(){
	tm.control_mode = MODE_NOMINAL;
	tm.learning_enabled = 0;
	tm.adaptation_enabled = 0;
	tm.trial_type = 0;

    tm.elapsed_samples = 0.0;
    tm.latest_foot_off_samples = 10000.0;
    tm.prev_stride_samples = 0.0;
    tm.in_swing = 0;
    tm.passed_min_stance_tq = 0;

    tm.gait_event_trigger = 0;  
    tm.stride_classified = 0;

    tm.tq = 0.0;
    tm.tq_dot = 0.0;
    tm.aa = 0.0;
    tm.aa_dot = 0.0;
    tm.aa_dot_15hz_filt = 0.0;

    tm.tq_prev = 0.0;
    tm.aa_prev = 0.0;

    tm.net_work_j = 0.0;
    tm.peak_power_w = 0.0;
    tm.min_power_w = 0.0;
    tm.stance_rom_rad = 0.0;
    tm.heelstrike_angle_rad = 0.0;

    tm.net_work_error_j_p_kg = (float*)calloc(N_CLASSES+1, sizeof(float));
    tm.stance_rom_error_rad = (float*)calloc(N_CLASSES+1, sizeof(float));
    tm.heelstrike_angle_error_rad = (float*)calloc(N_CLASSES+1, sizeof(float));

    tm.subject_mass_kg = 82.0;

}

//Determines whether we are in swing or in stance in a way that performs well across terrains.
static void update_gait_events(Act_s* actx){

    tm.gait_event_trigger = GAIT_EVENT_DEFAULT;

    if (tm.in_swing){

          //Prevent transition to stance until certain time has elapsed.
          if(tm.elapsed_samples - tm.latest_foot_off_samples < MIN_GAIT_PHASE_SAMPLES){
          	  return;
          }

          //To transition to stance, detect a high absolute tq derivative and a significant negative power
          // (negative power important to prevent triggering stance when servoing in free space).
          if (fabs(tm.tq_dot) >= MAX_SWING_TQ_DOT_NM_HZ && tm.power_w < TRANSITION_POWER_THRESH_W){
        	  tm.prev_stride_samples = tm.elapsed_samples;
              tm.elapsed_samples = 0;
              tm.in_swing = 0;
              tm.gait_event_trigger = GAIT_EVENT_FOOT_ON;
              tm.passed_min_stance_tq = 0;
          }
    }else{

    	//Prevent transition to swing until certain time has elapsed.
    	if (tm.elapsed_samples < MIN_GAIT_PHASE_SAMPLES){
			return;
    	}

    	//Require passing a certain minimum tq value before transition to stance is allowed.
		if (tm.tq > MIN_STANCE_TQ_NM){
			tm.passed_min_stance_tq = 1;
		}

		//To transition to swing, torque has to be within proximity to a defined swing baseline (here it is
		//EXPECTED_SWING_TQ_NM). This is currently a variable because with encoder-based tq sensing sometimes
		//there is drift in the baseline. Most of the time EXPECTED_SWING_TQ_NM = 0 though.
		if (fabs(EXPECTED_SWING_TQ_NM - tm.tq) < TRANSITION_TQ_THRESH_NM &&
		  tm.passed_min_stance_tq){
			tm.in_swing = 1;
			tm.latest_foot_off_samples = tm.elapsed_samples;
			tm.gait_event_trigger = GAIT_EVENT_FOOT_OFF;
			tm.stride_classified = 0;
		}
      }

}

//Calculates ankle power and work, resetting work at the beginning of every stance period.
static void update_ankle_dynamics(Act_s* actx)
{
    tm.tq_prev = tm.tq;
    tm.aa_prev = tm.aa;
    //
	#if defined(NO_DEVICE)
    simulate_ankle_torque();
    tm.tq = tqraw; //for debug with no actuator
    tm.aa = aaraw;
	#else
    tm.tq = actx->jointTorque;
    tm.aa = actx->jointAngle;
	#endif

    tm.tq_dot = filter_second_order_butter_50hz(SAMPLE_RATE_HZ*(tm.tq - tm.tq_prev), &tq_dot_outputs[0], &tq_dot_inputs[0]);
    tm.aa_dot = filter_second_order_butter_50hz(SAMPLE_RATE_HZ*(tm.aa - tm.aa_prev), &aa_dot_outputs[0], &aa_dot_inputs[0]);

    tm.power_w = tm.tq*tm.aa_dot;
    tm.net_work_j = tm.net_work_j + tm.power_w*SAMPLE_PERIOD_S;
	if (tm.peak_power_w < tm.power_w){
		tm.peak_power_w = tm.power_w;
	}
    if (tm.gait_event_trigger == GAIT_EVENT_FOOT_ON){
        tm.net_work_j = 0.0;
		tm.peak_power_w = -FLT_MAX;
    }

}

struct taskmachine_s* get_task_machine(){
  return &tm;
}


/* ---- Main task machine loop ---- */
// Handles all of the main functionality of Roman's terrain
// adaptive control system including estimating ankle dynamics, updating gait events,
// calculating kinematics and ground slope, using these measurements to
// heuristically determine the terrain, and then actuating the appropriate controller
// for the given terrain.
void task_machine_demux(struct rigid_s* rigid, Act_s* actx) {

	// Primary switch case
	// ???: Do we need all of these cases? why not just initialize everything in a single step?
	switch (task_machine_demux_state) {
		case INIT_TASK_MACHINE:
			init_task_machine();
			init_heuristics();
			task_machine_demux_state = INIT_KINEMATICS;
		break;
		case INIT_KINEMATICS:
			init_kinematics();
			task_machine_demux_state = INIT_TERRAIN_STATE_MACHINE;
		break;
		case INIT_TERRAIN_STATE_MACHINE:
			init_terrain_state_machine();
			task_machine_demux_state = RUN_TASK_MACHINE;
		break;
		case RUN_TASK_MACHINE:
			update_ankle_dynamics(actx);
			update_gait_events(actx);
			update_kinematics(&rigid->mn,&tm);
			update_heuristics(&tm, get_kinematics());


			//Use this switch to be in either terrain adaptive mode or just default to one steady-state controller. Depends
			//on which gui_mode you are in.
			if (tm.adaptation_enabled)
				tm.current_terrain = get_heuristics()->prediction;
			else
				tm.current_terrain = tm.control_mode;

			terrain_state_machine_demux(&tm, rigid, actx, tm.current_terrain);


			tm.elapsed_samples = tm.elapsed_samples + 1.0;
		break;
	}

  

}
