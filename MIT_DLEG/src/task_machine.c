 

#include "task_machine.h"

 //Gait event thresholds
#define EXPECTED_SWING_TQ 0.0
#define TRANSITION_TQ_THRESH 10.0
#define MIN_STANCE_TQ EXPECTED_SWING_TQ + TRANSITION_TQ_THRESH + 5.0
#define PREDICTION_CUTOFF_SAMPLES 200
#define MAX_SWING_TQ_DOT_NM_HZ 100
#define MIN_GAIT_PHASE_SAMPLES 250

//Ideal controller values
#define FL_IDEAL_NET_WORK_J_PER_KG 0.0244 //Sinitski 2012
#define US_IDEAL_NET_WORK_J_PER_KG 0.386 //Sinitski 2012
#define DS_IDEAL_NET_WORK_J_PER_KG -0.559 //Sinitski 2012
#define UR_IDEAL_NET_WORK_J_PER_KG 0.390 //calculated from digitized McIntosh 2006 ankle power
#define DR_IDEAL_NET_WORK_J_PER_KG -0.391 //calculated from digitized McIntosh 2006 ankle power
#define FL_IDEAL_ROM_RAD 0.51 //Sinitski 2012
#define US_IDEAL_ROM_RAD 0.74 //Sinitski 2012
#define DS_IDEAL_ROM_RAD 1.06 //Sinitski 2012
#define UR_IDEAL_ROM_RAD 0.642 //calculated from digitized McIntosh 2006 ankle angle
#define DR_IDEAL_ROM_RAD 0.398 //calculated from digitized McIntosh 2006 ankle angle
#define FL_IDEAL_FOOTSTRIKE_ANGLE_RAD 0.03 //Sinitski 2012
#define US_IDEAL_FOOTSTRIKE_ANGLE_RAD 0.28 //Sinitski 2012
#define DS_IDEAL_FOOTSTRIKE_ANGLE_RAD -0.51 //Sinitski 2012
#define UR_IDEAL_FOOTSTRIKE_ANGLE_RAD 0.14 //calculated from digitized McIntosh 2006 ankle angle
#define DR_IDEAL_FOOTSTRIKE_ANGLE_RAD 0.03 //calculated from digitized McIntosh 2006 ankle angle

static struct taskmachine_s tm;
static int task_machine_demux_state = INIT_TASK_MACHINE;
#if defined(NO_DEVICE)
static float tqraw;
static float aaraw;
float example_stride_tq[] = {0.03, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.01, 0.01, 0.01, -0.01, -0.01, -0.01, -0.01, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, -0.61, -0.61, -4.73, -4.73, -4.73, -4.73, -6.77, -6.77, -6.77, -6.77, -7.79, -7.79, -7.79, -7.79, -7.79, -7.79, -7.79, -8.04, -8.04, -8.04, -8.04, -8.04, -8.04, -8.04, -7.86, -7.86, -7.86, -7.32, -7.32, -7.32, -7.32, -5.96, -4.81, -4.81, -4.81, -3.64, -2.02, -2.02, -2.02, -2.02, -2.02, -2.02, -2.02, -2.02, -1.65, -1.65, -1.65, -1.65, -1.65, -1.65, -1.99, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -3.26, -3.26, -3.26, -3.26, -4.22, -4.22, -4.22, -4.22, -4.22, -4.22, -4.22, -5.94, -5.22, -5.22, -5.22, -3.83, -1.36, -1.36, -1.36, -1.36, -1.07, -1.07, -1.07, -1.07, -1.07, -1.07, -0.52, -0.52, -0.52, -0.52, 0.29, 0.29, 0.29, 0.29, 1.33, 2.61, 2.61, 2.61, 2.61, 2.61, 3.91, 3.91, 3.91, 3.91, 3.91, 3.91, 3.91, 5.10, 5.10, 5.10, 5.10, 5.10, 6.92, 7.64, 7.64, 7.64, 7.64, 7.64, 7.64, 8.37, 10.59, 10.59, 10.59, 11.25, 11.25, 11.87, 11.87, 11.87, 11.87, 11.87, 11.87, 13.02, 13.02, 13.02, 13.52, 13.52, 13.52, 13.94, 13.94, 13.94, 13.94, 13.94, 13.94, 13.94, 13.94, 13.94, 13.94, 14.26, 14.26, 14.54, 14.54, 14.54, 14.54, 14.54, 14.54, 14.54, 14.54, 14.54, 14.91, 14.91, 14.91, 14.96, 14.90, 14.90, 14.22, 14.22, 14.22, 14.22, 13.84, 13.84, 13.84, 13.84, 13.84, 13.84, 13.84, 13.84, 12.92, 12.42, 12.42, 12.42, 11.88, 11.88, 11.88, 11.88, 11.88, 11.88, 11.88, 11.35, 11.35, 11.35, 11.35, 11.35, 10.83, 10.83, 10.83, 10.83, 10.83, 10.83, 10.83, 10.30, 10.30, 9.30, 9.30, 9.30, 9.30, 8.34, 8.34, 7.48, 7.12, 7.12, 7.12, 6.83, 6.83, 6.83, 6.83, 6.83, 6.83, 6.83, 6.83, 6.53, 6.16, 6.16, 6.16, 6.16, 6.16, 6.02, 6.02, 5.90, 5.90, 5.90, 5.90, 5.90, 5.90, 5.90, 5.82, 5.82, 5.82, 5.82, 5.82, 5.82, 5.82, 5.77, 5.77, 5.77, 5.77, 5.77, 5.81, 5.81, 5.81, 5.81, 5.94, 6.25, 6.44, 6.44, 6.44, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.88, 6.88, 6.88, 6.88, 6.88, 6.88, 7.76, 7.76, 7.76, 7.76, 7.76, 7.76, 7.76, 7.76, 8.06, 8.06, 8.06, 8.06, 8.39, 8.39, 8.39, 8.39, 8.39, 9.06, 9.06, 9.06, 9.06, 9.06, 9.06, 9.06, 9.70, 9.70, 10.60, 10.60, 10.88, 10.88, 10.88, 10.88, 10.88, 10.88, 10.88, 10.88, 11.36, 11.36, 11.36, 11.59, 11.59, 11.59, 11.81, 11.81, 11.81, 12.04, 12.04, 12.04, 12.04, 12.04, 12.04, 12.04, 12.04, 12.29, 12.29, 12.29, 12.29, 12.29, 12.29, 12.29, 12.29, 12.74, 12.74, 12.74, 12.74, 12.74, 12.74, 12.96, 12.96, 13.82, 13.82, 14.09, 14.09, 14.09, 14.38, 14.38, 14.38, 14.90, 14.90, 14.90, 14.90, 15.36, 15.36, 15.36, 15.36, 15.36, 15.36, 15.36, 15.36, 15.36, 15.36, 15.36, 15.59, 15.59, 15.59, 15.82, 15.82, 15.82, 15.82, 15.82, 15.82, 16.25, 16.25, 16.25, 16.25, 16.25, 16.25, 16.67, 16.67, 17.18, 17.18, 17.18, 17.18, 17.34, 17.48, 17.48, 17.48, 17.48, 17.48, 17.48, 17.80, 17.80, 17.80, 17.80, 17.80, 17.93, 18.02, 18.02, 18.02, 18.02, 18.15, 18.15, 18.15, 18.15, 18.15, 18.24, 18.24, 18.24, 18.24, 18.24, 18.24, 18.24, 18.24, 18.24, 18.24, 18.37, 18.47, 18.47, 18.47, 18.47, 18.47, 18.61, 18.61, 18.72, 19.12, 19.12, 19.28, 19.28, 19.28, 19.28, 19.49, 19.49, 19.49, 19.49, 19.86, 19.86, 19.86, 19.86, 19.86, 19.86, 19.86, 19.86, 19.86, 19.86, 20.08, 20.08, 20.08, 20.33, 20.33, 20.60, 20.60, 20.60, 20.60, 20.84, 20.84, 20.84, 20.84, 20.84, 20.84, 20.84, 21.40, 21.70, 21.70, 21.70, 21.70, 21.70, 21.70, 22.34, 23.34, 23.34, 23.34, 23.34, 23.62, 23.62, 23.62, 23.62, 23.62, 23.62, 23.62, 24.25, 24.25, 24.25, 24.25, 24.25, 24.59, 24.59, 24.96, 24.96, 24.96, 25.32, 25.32, 25.32, 25.32, 25.32, 25.32, 25.32, 25.32, 25.66, 25.66, 25.66, 25.66, 25.66, 25.66, 25.66, 25.98, 25.98, 25.98, 25.98, 25.98, 25.98, 25.98, 25.98, 26.99, 26.99, 27.92, 27.92, 28.24, 28.24, 28.24, 28.24, 28.24, 28.82, 28.82, 28.82, 28.82, 28.82, 28.82, 29.12, 29.39, 29.39, 29.39, 29.39, 29.64, 29.64, 29.64, 29.64, 29.64, 29.64, 29.89, 29.89, 29.89, 29.89, 30.14, 30.14, 30.14, 30.14, 30.14, 30.64, 30.64, 30.64, 30.64, 30.64, 30.64, 30.86, 31.08, 31.52, 31.52, 31.52, 31.52, 31.73, 31.73, 31.94, 31.94, 31.94, 32.17, 32.17, 32.40, 32.62, 32.62, 32.62, 32.62, 32.84, 32.84, 33.08, 33.08, 33.08, 33.08, 33.08, 33.08, 33.08, 33.35, 33.35, 33.35, 33.35, 33.62, 33.62, 33.62, 33.62, 33.62, 33.62, 33.62, 33.62, 34.15, 34.15, 34.15, 34.15, 34.15, 34.64, 34.64, 34.64, 35.48, 35.48, 35.76, 35.76, 35.76, 35.76, 36.04, 36.04, 36.04, 36.04, 36.04, 36.66, 36.66, 36.66, 36.66, 36.66, 36.66, 36.66, 36.92, 36.92, 37.21, 37.21, 37.52, 37.52, 37.52, 37.52, 37.52, 37.52, 37.52, 37.85, 37.85, 37.85, 37.85, 37.85, 38.21, 38.21, 38.21, 38.21, 38.85, 38.85, 38.85, 38.85, 38.85, 38.85, 39.47, 39.47, 40.44, 40.44, 40.44, 40.75, 40.75, 40.75, 40.75, 40.75, 40.75, 40.75, 41.07, 41.66, 41.66, 41.66, 41.66, 41.66, 41.66, 41.93, 42.20, 42.20, 42.20, 42.20, 42.20, 42.20, 42.20, 42.20, 42.20, 42.20, 42.45, 42.45, 42.68, 42.68, 42.68, 42.68, 42.68, 42.68, 42.68, 42.68, 42.92, 42.92, 42.92, 42.92, 42.92, 43.56, 43.56, 44.26, 44.26, 44.26, 44.51, 44.51, 44.51, 44.51, 44.77, 44.77, 44.77, 45.04, 45.04, 45.33, 45.33, 45.33, 45.33, 45.61, 45.92, 45.92, 45.92, 45.92, 45.92, 45.92, 45.92, 45.92, 45.92, 45.92, 45.92, 45.92, 46.22, 46.22, 46.55, 46.55, 46.55, 46.55, 46.55, 47.24, 49.30, 49.30, 49.30, 49.30, 49.64, 50.00, 50.00, 50.00, 51.09, 51.09, 51.43, 51.43, 51.43, 51.43, 51.43, 51.43, 51.43, 51.76, 51.76, 51.76, 51.76, 51.76, 51.76, 51.76, 51.76, 51.76, 51.76, 51.76, 51.76, 51.76, 52.07, 52.07, 52.07, 52.07, 52.07, 52.07, 52.07, 52.63, 52.63, 52.63, 52.63, 52.63, 52.63, 53.02, 53.02, 53.02, 53.02, 53.02, 53.02, 53.02, 53.02, 53.02, 53.02, 53.14, 53.14, 53.18, 53.18, 53.18, 53.18, 53.18, 53.18, 53.18, 53.18, 53.18, 53.18, 53.09, 52.97, 52.97, 52.97, 52.97, 52.97, 52.97, 52.45, 52.45, 51.22, 51.22, 51.22, 51.22, 50.73, 50.73, 50.73, 50.33, 50.33, 50.33, 50.33, 49.52, 49.52, 49.52, 49.52, 49.52, 49.52, 49.03, 48.47, 48.47, 48.47, 48.47, 48.47, 48.47, 48.47, 48.47, 48.47, 48.47, 48.47, 47.82, 47.82, 47.82, 47.11, 47.11, 47.11, 47.11, 47.11, 45.49, 45.49, 45.49, 45.49, 45.49, 44.56, 43.57, 40.28, 40.28, 40.28, 40.28, 40.28, 39.10, 37.80, 37.80, 37.80, 37.80, 37.80, 35.11, 35.11, 35.11, 33.71, 33.71, 33.71, 33.71, 32.26, 32.26, 30.80, 30.80, 30.80, 30.80, 30.80, 30.80, 30.80, 30.80, 30.80, 30.80, 29.28, 29.28, 29.28, 27.75, 27.75, 27.75, 27.75, 27.75, 24.69, 24.69, 24.69, 23.18, 23.18, 23.18, 23.18, 21.68, 21.68, 17.31, 17.31, 17.31, 15.90, 15.90, 15.90, 14.50, 14.50, 14.50, 14.50, 14.50, 14.50, 13.10, 13.10, 13.10, 13.10, 13.10, 13.10, 10.40, 10.40, 10.40, 9.10, 9.10, 9.10, 9.10, 9.10, 9.10, 9.10, 9.10, 9.10, 7.81, 7.81, 7.81, 7.81, 7.81, 7.81, 6.52, 6.52, 6.52, 6.52, 6.52, 6.52, 2.79, 2.79, 0.10, 0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.09, -0.09, -0.09, -0.09, -0.09, -0.09, -0.09, -0.09, 0.00, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.08, -0.08, -0.08, -0.08, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.11, -0.11, -0.11, -0.11, -0.11, -0.13, -0.13, -0.06, -0.05, -0.05, -0.05, -0.05, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.08, -0.08, -0.08, -0.08, -0.08, -0.08, -0.08, -0.08, -0.08, -0.08, -0.08, -0.09, -0.09, -0.09, -0.09, -0.11, -0.11, -0.11, -0.11, -0.11, -0.11, -0.11, -0.11, -0.11, -0.09, -0.09, -0.12, -0.12, -0.13, -0.13, -0.13, -0.13, -0.13, -0.13, -0.13, -0.13, -0.13, -0.13, -0.13, -0.13, -0.10, -0.10, -0.09, -0.09, -0.09, -0.09, -0.09, -0.09, -0.09, -0.09, -0.09, -0.08, -0.08, -0.06, -0.06, -0.06, -0.06, -0.06, -0.06, -0.06, -0.06, -0.06, -0.06, -0.06, -0.06, -0.06, -0.06, -0.07, -0.07, -0.09, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.10, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.07, -0.03, -0.03, -0.03, -0.03, -0.03, -0.06, -0.09, -0.06, -0.06, -0.06, -0.06, -0.06, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, -0.04, 0.01, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.03, -0.03, -0.03, -0.02, -0.02, -0.02, -0.02, -0.02, -0.02, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.01, -0.02, -0.02, -0.02, -0.02, -0.02, 0.00, 0.00, 0.00, 0.00, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.02, 0.02, 0.02, 0.02, 0.02, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.08, 0.08, 0.08, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.07, 0.07, 0.07, 0.11, 0.11, 0.11, 0.10, 0.10, 0.10, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.13, 0.13, 0.13, 0.13, 0.13, 0.13, 0.13, 0.13, 0.13, 0.13, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.14, 0.14, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.14, 0.14, 0.14, 0.14, 0.14, 0.13, 0.13, 0.13, 0.13, 0.13, 0.13, 0.13, 0.13, 0.13, 0.13, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.09, 0.09, 0.15, 0.15, 0.15, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.11, 0.07, 0.07, 0.07, 0.07, 0.07, 0.07, 0.07, 0.07, 0.07, 0.07, 0.07, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.07, 0.07, 0.07, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.13, 0.09, 0.09, 0.10, 0.10, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09, 0.12, 0.11, 0.11, 0.11, 0.11, 0.08, 0.08, 0.08, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.03, 0.03, 0.03, 0.03, 0.03, 0.03, 0.03};
float example_stride_aa[] = {0.15, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.19, 0.19, 0.51, 0.51, 0.51, 0.51, 0.72, 0.72, 0.72, 0.72, 0.84, 0.84, 0.84, 0.84, 0.84, 0.84, 0.84, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.83, 0.83, 0.83, 0.77, 0.77, 0.77, 0.77, 0.73, 0.66, 0.66, 0.66, 0.58, 0.48, 0.48, 0.48, 0.48, 0.48, 0.48, 0.48, 0.48, 0.46, 0.50, 0.50, 0.50, 0.50, 0.50, 0.55, 0.61, 0.61, 0.61, 0.61, 0.61, 0.61, 0.61, 0.61, 0.61, 0.68, 0.68, 0.68, 0.68, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.95, 0.95, 0.95, 0.95, 0.84, 0.62, 0.62, 0.62, 0.62, 0.63, 0.63, 0.63, 0.63, 0.63, 0.63, 0.60, 0.60, 0.60, 0.60, 0.53, 0.53, 0.53, 0.53, 0.43, 0.31, 0.31, 0.31, 0.31, 0.31, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.06, 0.06, 0.06, 0.06, 0.06, -0.09, -0.15, -0.15, -0.15, -0.15, -0.15, -0.15, -0.20, -0.39, -0.39, -0.39, -0.46, -0.46, -0.52, -0.52, -0.52, -0.52, -0.52, -0.52, -0.68, -0.68, -0.68, -0.75, -0.75, -0.75, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.92, -0.92, -1.00, -1.00, -1.00, -1.00, -1.00, -1.00, -1.00, -1.00, -1.00, -1.19, -1.19, -1.19, -1.27, -1.37, -1.37, -1.65, -1.65, -1.65, -1.65, -1.75, -1.75, -1.75, -1.75, -1.75, -1.75, -1.75, -1.75, -1.94, -2.04, -2.04, -2.04, -2.13, -2.13, -2.13, -2.13, -2.13, -2.13, -2.13, -2.22, -2.22, -2.22, -2.22, -2.22, -2.30, -2.30, -2.30, -2.30, -2.30, -2.30, -2.30, -2.37, -2.37, -2.50, -2.50, -2.50, -2.50, -2.61, -2.61, -2.68, -2.70, -2.70, -2.70, -2.72, -2.72, -2.72, -2.72, -2.72, -2.72, -2.72, -2.72, -2.73, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.74, -2.73, -2.73, -2.73, -2.73, -2.73, -2.73, -2.73, -2.71, -2.71, -2.71, -2.71, -2.71, -2.66, -2.66, -2.66, -2.66, -2.63, -2.59, -2.56, -2.56, -2.56, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.51, -2.51, -2.51, -2.51, -2.51, -2.51, -2.46, -2.46, -2.46, -2.46, -2.46, -2.46, -2.46, -2.46, -2.45, -2.45, -2.45, -2.45, -2.44, -2.44, -2.44, -2.44, -2.44, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.40, -2.40, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.41, -2.43, -2.43, -2.43, -2.46, -2.46, -2.46, -2.48, -2.48, -2.48, -2.51, -2.51, -2.51, -2.51, -2.51, -2.51, -2.51, -2.51, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.54, -2.60, -2.60, -2.60, -2.60, -2.60, -2.60, -2.64, -2.64, -2.75, -2.75, -2.79, -2.79, -2.79, -2.83, -2.83, -2.83, -2.89, -2.89, -2.89, -2.89, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -2.97, -3.01, -3.01, -3.01, -3.04, -3.04, -3.04, -3.04, -3.04, -3.04, -3.10, -3.10, -3.10, -3.10, -3.10, -3.10, -3.17, -3.17, -3.29, -3.29, -3.29, -3.29, -3.35, -3.39, -3.39, -3.39, -3.39, -3.39, -3.39, -3.46, -3.46, -3.46, -3.46, -3.46, -3.51, -3.56, -3.56, -3.56, -3.56, -3.62, -3.62, -3.62, -3.62, -3.62, -3.67, -3.67, -3.67, -3.67, -3.67, -3.67, -3.67, -3.67, -3.67, -3.67, -3.71, -3.77, -3.77, -3.77, -3.77, -3.77, -3.83, -3.83, -3.88, -4.03, -4.03, -4.08, -4.08, -4.08, -4.08, -4.12, -4.12, -4.12, -4.12, -4.23, -4.23, -4.23, -4.23, -4.23, -4.23, -4.23, -4.23, -4.23, -4.23, -4.28, -4.28, -4.28, -4.32, -4.32, -4.38, -4.38, -4.38, -4.38, -4.43, -4.43, -4.43, -4.43, -4.43, -4.43, -4.43, -4.54, -4.58, -4.58, -4.58, -4.58, -4.58, -4.58, -4.66, -4.82, -4.82, -4.82, -4.82, -4.87, -4.87, -4.87, -4.87, -4.87, -4.87, -4.87, -4.99, -4.99, -4.99, -4.99, -4.99, -5.05, -5.05, -5.11, -5.11, -5.11, -5.16, -5.16, -5.16, -5.16, -5.16, -5.16, -5.16, -5.16, -5.21, -5.21, -5.21, -5.21, -5.21, -5.21, -5.21, -5.26, -5.26, -5.26, -5.26, -5.26, -5.26, -5.26, -5.26, -5.43, -5.43, -5.59, -5.59, -5.66, -5.66, -5.66, -5.66, -5.66, -5.79, -5.79, -5.79, -5.79, -5.79, -5.79, -5.85, -5.92, -5.92, -5.92, -5.92, -5.99, -5.99, -5.99, -5.99, -5.99, -5.99, -6.06, -6.06, -6.06, -6.06, -6.12, -6.12, -6.12, -6.12, -6.12, -6.27, -6.27, -6.27, -6.27, -6.27, -6.27, -6.33, -6.41, -6.54, -6.54, -6.54, -6.54, -6.61, -6.61, -6.69, -6.69, -6.69, -6.76, -6.76, -6.84, -6.91, -6.91, -6.91, -6.91, -6.98, -6.98, -7.06, -7.06, -7.06, -7.06, -7.06, -7.06, -7.06, -7.12, -7.12, -7.12, -7.12, -7.19, -7.19, -7.19, -7.19, -7.19, -7.19, -7.19, -7.19, -7.34, -7.34, -7.34, -7.34, -7.34, -7.50, -7.50, -7.50, -7.69, -7.69, -7.76, -7.76, -7.76, -7.76, -7.83, -7.83, -7.83, -7.83, -7.83, -7.97, -7.97, -7.97, -7.97, -7.97, -7.97, -7.97, -8.04, -8.04, -8.10, -8.10, -8.17, -8.17, -8.17, -8.17, -8.17, -8.17, -8.17, -8.23, -8.23, -8.23, -8.23, -8.23, -8.30, -8.30, -8.30, -8.30, -8.45, -8.45, -8.45, -8.45, -8.45, -8.45, -8.59, -8.59, -8.83, -8.83, -8.83, -8.91, -8.91, -8.91, -8.91, -8.91, -8.91, -8.91, -8.99, -9.13, -9.13, -9.13, -9.13, -9.13, -9.13, -9.20, -9.27, -9.27, -9.27, -9.27, -9.27, -9.27, -9.27, -9.27, -9.27, -9.27, -9.34, -9.34, -9.43, -9.43, -9.43, -9.43, -9.43, -9.43, -9.43, -9.43, -9.50, -9.50, -9.50, -9.50, -9.50, -9.75, -9.75, -9.98, -9.98, -9.98, -10.05, -10.05, -10.05, -10.05, -10.14, -10.14, -10.14, -10.22, -10.22, -10.31, -10.31, -10.31, -10.31, -10.39, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.48, -10.57, -10.57, -10.65, -10.65, -10.65, -10.65, -10.65, -10.82, -11.32, -11.32, -11.32, -11.32, -11.40, -11.48, -11.48, -11.48, -11.71, -11.71, -11.79, -11.79, -11.79, -11.79, -11.79, -11.79, -11.79, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.87, -11.95, -11.95, -11.95, -11.95, -11.95, -11.95, -11.95, -12.10, -12.10, -12.10, -12.10, -12.10, -12.10, -12.23, -12.23, -12.23, -12.23, -12.23, -12.23, -12.23, -12.23, -12.23, -12.23, -12.30, -12.30, -12.36, -12.36, -12.36, -12.36, -12.36, -12.36, -12.36, -12.36, -12.36, -12.36, -12.42, -12.46, -12.46, -12.46, -12.46, -12.46, -12.46, -12.53, -12.53, -12.59, -12.59, -12.59, -12.59, -12.61, -12.61, -12.61, -12.63, -12.63, -12.63, -12.63, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.68, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.67, -12.65, -12.65, -12.65, -12.60, -12.60, -12.60, -12.60, -12.60, -12.52, -12.52, -12.52, -12.52, -12.52, -12.46, -12.38, -12.13, -12.13, -12.13, -12.13, -12.13, -12.03, -11.92, -11.92, -11.92, -11.92, -11.92, -11.69, -11.69, -11.69, -11.56, -11.56, -11.56, -11.56, -11.42, -11.42, -11.28, -11.28, -11.28, -11.28, -11.28, -11.28, -11.28, -11.28, -11.28, -11.28, -11.13, -11.13, -11.13, -10.97, -10.97, -10.97, -10.97, -10.97, -10.65, -10.65, -10.65, -10.46, -10.46, -10.46, -10.46, -10.26, -10.26, -9.65, -9.65, -9.65, -9.43, -9.43, -9.43, -9.20, -9.20, -9.20, -9.20, -9.20, -9.20, -8.96, -8.96, -8.96, -8.96, -8.96, -8.96, -8.46, -8.46, -8.46, -8.19, -8.19, -8.19, -8.19, -8.19, -8.19, -8.19, -8.19, -8.19, -7.92, -7.92, -7.92, -7.92, -7.92, -7.92, -7.64, -7.64, -7.64, -7.64, -7.64, -7.64, -6.69, -6.69, -5.68, -5.68, -5.39, -5.39, -5.39, -5.39, -5.39, -5.15, -5.15, -5.15, -5.15, -5.15, -5.15, -5.15, -5.15, -4.66, -4.13, -4.13, -4.13, -4.13, -4.13, -4.13, -4.13, -4.13, -4.13, -4.13, -3.85, -3.85, -3.85, -3.85, -3.59, -3.59, -3.59, -3.59, -3.59, -3.59, -3.33, -3.33, -3.33, -3.33, -3.33, -3.06, -3.06, -2.03, -1.79, -1.79, -1.79, -1.79, -1.55, -1.55, -1.55, -1.55, -1.55, -1.55, -1.55, -1.55, -1.55, -0.93, -0.76, -0.76, -0.76, -0.76, -0.76, -0.76, -0.76, -0.76, -0.76, -0.76, -0.63, -0.63, -0.63, -0.63, -0.49, -0.49, -0.49, -0.49, -0.49, -0.49, -0.49, -0.49, -0.49, -0.11, -0.11, 0.13, 0.13, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.23, 0.23, 0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.34, 0.35, 0.35, 0.35, 0.35, 0.34, 0.34, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.30, 0.30, 0.30, 0.30, 0.30, 0.30, 0.30, 0.30, 0.30, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.25, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.22, 0.22, 0.22, 0.23, 0.23, 0.23, 0.23, 0.23, 0.23, 0.23, 0.23, 0.23, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.19, 0.19, 0.19, 0.19, 0.19, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.21, 0.21, 0.21, 0.21, 0.21, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.19, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.17, 0.17, 0.17, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.16, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.16, 0.16, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17, 0.17};
#endif
static float ideal_net_work_j_per_kg[] = {FL_IDEAL_NET_WORK_J_PER_KG,UR_IDEAL_NET_WORK_J_PER_KG,DR_IDEAL_NET_WORK_J_PER_KG,US_IDEAL_NET_WORK_J_PER_KG,DS_IDEAL_NET_WORK_J_PER_KG};
static float ideal_rom_rad[] = {FL_IDEAL_ROM_RAD,UR_IDEAL_ROM_RAD,DR_IDEAL_ROM_RAD,US_IDEAL_ROM_RAD,DS_IDEAL_ROM_RAD};
static float ideal_heelstrike_angle_rad[] = {FL_IDEAL_FOOTSTRIKE_ANGLE_RAD,UR_IDEAL_FOOTSTRIKE_ANGLE_RAD,DR_IDEAL_FOOTSTRIKE_ANGLE_RAD,US_IDEAL_FOOTSTRIKE_ANGLE_RAD,DS_IDEAL_FOOTSTRIKE_ANGLE_RAD};

static float aa_dot_15hz_filt_outputs[] = {0,0,0,0,0};
static float aa_dot_15hz_filt_inputs[] = {0,0,0,0,0};

static float tq_dot_outputs[] = {0,0,0};
static float tq_dot_inputs[] = {0,0,0};
static float aa_dot_outputs[] = {0,0,0};
static float aa_dot_inputs[] = {0,0,0};

//static float torque_replay_70kg[] = {-4.001649, -4.852673, -11.852532, -14.614554, -9.826756, -8.796879, -5.918001, -0.888233, 2.200429, 5.075491, 8.681907, 12.352409, 15.634863, 19.046701, 22.175614, 25.030629, 27.762032, 30.134923, 32.248197, 34.176628, 35.853156, 37.183527, 38.377596, 39.389298, 40.306929, 41.285400, 42.317727, 43.574136, 45.059061, 46.821967, 48.937716, 51.406733, 54.387221, 57.872666, 61.833998, 66.243208, 71.179092, 76.448118, 81.949901, 87.699807, 93.607025, 99.355844, 104.744118, 109.791961, 113.980215, 117.084070, 118.982611, 119.405260, 117.985780, 114.244175, 107.925739, 100.368482, 90.742188, 76.392282, 59.252296, 40.923735, 23.919683, 10.275839, 0.948522, -2.241754, -2.738065, -2.837141, -2.723922, -2.406633, -1.957389, -1.487492, -1.102087, -0.857047, -0.747952, -0.729212, -0.747070, -0.765693, -0.773913, -0.776564, -0.783944, -0.804860, -0.842396, -0.893250, -0.949676, -1.004377, -1.055209, -1.105390, -1.161034, -1.226084, -1.294344, -1.346250, -1.355576, -1.296739, -1.149954, -0.909682, -0.587207, -0.206126, 0.196165, 0.580245, 0.911793, 1.160153, 1.293800, 1.279092, 1.080700, 0.663608, 0.016374};
//static float stance_torque_replay_70kg[] = {-4.001649, -4.852673, -11.852532, -14.614554, -9.826756, -8.796879, -5.918001, -0.888233, 2.200429, 5.075491, 8.681907, 12.352409, 15.634863, 19.046701, 22.175614, 25.030629, 27.762032, 30.134923, 32.248197, 34.176628, 35.853156, 37.183527, 38.377596, 39.389298, 40.306929, 41.285400, 42.317727, 43.574136, 45.059061, 46.821967, 48.937716, 51.406733, 54.387221, 57.872666, 61.833998, 66.243208, 71.179092, 76.448118, 81.949901, 87.699807, 93.607025, 99.355844, 104.744118, 109.791961, 113.980215, 117.084070, 118.982611, 119.405260, 117.985780, 114.244175, 107.925739, 100.368482, 90.742188, 76.392282, 59.252296, 40.923735, 23.919683, 10.275839};
//static int torque_replay_transition_gait_cycle_percent = 57;
//static float prev_stride_tics = 1000.0;
//static float prev_stance_tics = 600.0;
//static float subject_weight_kg = 9.1;

//Copied from matlab pil simulation
static void init_task_machine(){
	tm.control_mode = MODE_NOMINAL;

    tm.elapsed_samples = 0.0;
    tm.latest_foot_off_samples = 10000.0;
    tm.in_swing = 0;
    tm.do_learning_for_curr_stride = 0;
    tm.do_learning_for_prev_stride = 0;
    tm.passed_min_stance_tq = 0;

    tm.gait_event_trigger = 0;  
    tm.reset_back_estimator_trigger = 0;
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


}



//Copied over from pil simulation
static void update_gait_events(Act_s* actx){

    tm.gait_event_trigger = GAIT_EVENT_DEFAULT;

    if (tm.in_swing){

          if (tm.elapsed_samples - tm.latest_foot_off_samples == PREDICTION_CUTOFF_SAMPLES){
              tm.do_learning_for_curr_stride = 1;
              tm.gait_event_trigger = GAIT_EVENT_WINDOW_CLOSE; 
              return;  
          }

          if(tm.elapsed_samples - tm.latest_foot_off_samples < MIN_GAIT_PHASE_SAMPLES){
          	  return;
          }

          if (fabs(tm.aa - actx->thetaDes) > 0.05)
        	  return;

            //Swing to stance transition condition
          if (fabs(EXPECTED_SWING_TQ - tm.tq) >= TRANSITION_TQ_THRESH ||
        		  fabs(tm.tq_dot) >= MAX_SWING_TQ_DOT_NM_HZ){
//        	  prev_stride_tics = tm.elapsed_samples;
              tm.elapsed_samples = 0;
              tm.in_swing = 0;
              tm.gait_event_trigger = GAIT_EVENT_FOOT_ON;
              tm.passed_min_stance_tq = 0;
          }
    }else{

    	if (tm.elapsed_samples < MIN_GAIT_PHASE_SAMPLES){
			return;
    	}
		if (tm.tq > MIN_STANCE_TQ){
			tm.passed_min_stance_tq = 1;
		}


		if (fabs(EXPECTED_SWING_TQ - tm.tq) < TRANSITION_TQ_THRESH &&
		  tm.passed_min_stance_tq){
//			prev_stance_tics = tm.elapsed_samples;
			tm.in_swing = 1;
			tm.latest_foot_off_samples = tm.elapsed_samples;
			tm.do_learning_for_prev_stride = tm.do_learning_for_curr_stride;
			tm.do_learning_for_curr_stride = 0;
			tm.gait_event_trigger = GAIT_EVENT_FOOT_OFF;
			tm.stride_classified = 0;
		}
      }

}

#if defined(NO_DEVICE)
static void simulate_ankle_torque(){
	static int stride_iterator = 0;
  tqraw = example_stride_tq[stride_iterator];
  aaraw = example_stride_aa[stride_iterator];
  stride_iterator++;
  if (stride_iterator == 1431)
    stride_iterator = 0;
}
#endif


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

   tm.aa_dot_15hz_filt = filter_fourth_order_butter_15hz( tm.aa_dot, &aa_dot_15hz_filt_outputs[0], &aa_dot_15hz_filt_inputs[0]);

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



void task_machine_demux(struct rigid_s* rigid, Act_s* actx){

  
  switch (task_machine_demux_state){
    case INIT_TASK_MACHINE:
        init_task_machine();
        init_back_estimator();
        task_machine_demux_state = INIT_LEARNING;
    break;
    case INIT_LEARNING:
        init_learning_structs();
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
		update_statistics_demux(&tm, get_kinematics());


		if (tm.control_mode == MODE_NOMINAL){

			update_learner_demux();
		}

		predict_task_demux(&tm, get_kinematics());

		update_back_estimation_features(&tm, get_kinematics());
		update_prediction_features(&tm, get_kinematics());


		if (tm.control_mode == MODE_ADAPTIVE_WITH_LEARNING){
			tm.current_terrain = get_predictor()->k_pred;
		}
		else if (tm.control_mode == MODE_ADAPTIVE_WITH_HEURISTICS){
				if (get_back_estimator()->curr_stride_paz_thresh_status == PAZ_PASSED_US_THRESH &&
						get_back_estimator()->curr_stride_paz_thresh_pass_samples <= 200){
					tm.current_terrain = MODE_USTAIRS;
				}
				else if (get_back_estimator()->curr_stride_paz_thresh_status == PAZ_PASSED_DS_THRESH &&
						get_back_estimator()->curr_stride_paz_thresh_pass_samples <= 200){
					tm.current_terrain = MODE_DSTAIRS;
				}
				else
				{
					tm.current_terrain = MODE_FLAT;
				}
			}else{
				tm.current_terrain = tm.control_mode;
		}
		terrain_state_machine_demux(&tm, rigid, actx, tm.current_terrain);


    	tm.elapsed_samples = tm.elapsed_samples + 1.0;
    	break;
  }

  

}
