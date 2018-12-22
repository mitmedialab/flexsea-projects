 

#include "task_machine.h"


static struct taskmachine_s tm;
static struct back_estimator_s be;

//Copied from matlab pil simulation
static void init_task_machine(){

    tm.latest_foot_static_samples = 0;
    tm.elapsed_samples = 0
    tm.latest_foot_off_samples = 10000;
    tm.in_swing = 0;
    tm.stride_classified = 0;
    tm.do_learning_for_stride = 0;

    tm.geit_event_trigger = 0;  
    tm.reset_back_estimator_trigger = 0;

    tm.low_torque_counter = 0;

    tm.tq = 0;
    tm.tq_dot = 0;
    tm.aa = 0;
    tm.aa_dot = 0;
    tm.aa_dot_aOmegaX_error = 0;

    tm.tq_prev = 0;
    tm.aa_prev = 0;
    tm.aa_dot_aOmegaX_error_prev = 0;

}

//Copied from matlab pil simulation
static void init_back_estimator(){
    be.sum_sin_sq_attack_angle = 0;
    be.prev_mean_sin_sq_attack_angle = 0;
    be.min_stance_theta = 0;
    be.max_stance_theta = 0;
    be.prev_min_stance_theta = 0;
    be.prev_passed_ds_z_thresh_samples = 0;
    be.passed_ds_z_thresh_samples = 0;
    be.passed_us_z_thresh = 0;
    be.passed_ds_z_thresh = 0;
    be.completed_back_estimation = 0;
}

//Copied over from pil simulation
static void update_gait_events(){

    tm.gait_event_trigger = 0;

    tm.tq_prev = tm.tq;
    tm.aa_prev = tm.aa;
    tm.aa_dot_aOmegaX_error_prev = tm.aa_dot_aOmegaX_error;
    tm.tq = FILTA*tm.tq + FILTB*sv.torqueRaw;
    tm.tq_dot = FILTA * tm.tq_dot + FILTB* (tm.tq - tm.tq_prev);
    tm.aa = FILTA*tm.aa + FILTB*sv.angleRaw;
    tm.aa_dot = FILTA*tm.aa_dot + FILTB*(tm.aa - tm.aa_prev)*17.45329;
    tm.aa_dot_aOmegaX_error = FILTA *tm.aa_dot_aOmegaX_error + FILTB*(tm.aa_dot - get_kinematics()->aOmegaX);

    if (tm.in_swing){

          if (tm.elapsed_samples - tm.latest_foot_off_samples >= PREDICTION_CUTOFF_SAMPLES && !tm.stride_classified){
              tm.do_learning_for_stride = 1; 
              tm.gait_event_trigger = GAIT_EVENT_WINDOW_CLOSE; 
              return;  
          }
            //Swing to stance transition condition
          if (fabs(tm.tq) >= MIN_TQ_FOR_FOOT_ON){
              tm.elapsed_samples = 0;
              tm.in_swing = 0;
              tm.low_torque_counter = 0;
              tm.gait_event_trigger = GAIT_EVENT_FOOT_ON; 
              tm.latest_foot_static_samples = tm.elapsed_samples;
          }
    }
    else{
          float abserr = fabs(tm.aa_dot_aOmegaX_error);
          float abstq = fabs(tm.tq);
          if (abserr < fabs(tm.aa_dot_aOmegaX_error_prev) &&
              abserr < AA_DOT_AOMEGA_ERROR_THRESH && abstq > MIN_TQ_FOR_FOOT_STATIC){
            tm.gait_event_trigger = GAIT_EVENT_FOOT_STATIC;
            tm.latest_foot_static_samples = elapsed_samples;
          }

          if (abstq < MIN_TQ_FOR_FOOT_ON)
            tm.low_torque_counter = tm.low_torque_counter + 1;
          else
            tm.low_torque_counter = 0;

          //Stance to swing transition condition
          if (tm.low_torque_counter > MIN_LOW_TQ_SAMPLES_FOR_SWING_TRANSITION){
            tm.in_swing = 1;
            tm.latest_foot_off_samples = tm.elapsed_samples;
            tm.stride_classified = 0;
            tm.gait_event_trigger = GAIT_EVENT_FOOT_OFF;    
          }
      }

}

static void simulate_ankle_torque(){
  if (tm.elapsed_samples % 2000 < 1000)
    tm.tq = 100.0f;
  else
    tm.tq = 0.0f;
}

static int task_machine_demux_state = 0;

struct taskmachine_s* get_task_machine(){
  return &tm;
}

void task_machine_demux(struct rigid_s* rigid){

  
  switch (tm.demux_state){
    case INIT_TASK_MACHINE:
        init_task_machine();
        init_back_estimator();
        tm.demux_state = INIT_LEARNING;
    break;
    case INIT_LEARNING:
        init_learning_structs();
        tm.demux_state = INIT_KINEMATICS;
    break;
    case INIT_KINEMATICS:
        init_kinematics();
        tm.demux_state = RUN_TASK_MACHINE;
    break;
    case RUN_TASK_MACHINE:


    update_gait_events();
    // update_kinematics(sv, tm,kin, cn, i);
    // update_learner_demux(tm, lrn, lda, prevfeats, be, cn, i);
    // update_classifier_demux(lda,lrn,cn);
    // update_back_estimation_features(tm,kin, be, cn, i);
    // update_prediction_features(tm,kin,currfeats,cn,i);
    // predict_task(currfeats, prevfeats, tm, kin, lda, cn, i);


        
    // state_machine_demux(rigid, get_classifier()->k_pred);

    break;
  }

  tm.elapsed_samples++;

}
