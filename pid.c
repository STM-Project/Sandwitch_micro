/*
 * pid.c
 *
 *  Created on: 19.03.2018
 *      Author: Elektronika RM
 */


#include "stm32f1xx_hal.h"
#include "pid.h"


/**
   * @brief  Process function for the Q31 PID Control.
   * @param[in,out] S  points to an instance of the Q31 PID Control structure
   * @param[in]     in  input sample to process
   * @return out processed output sample.
   *
   * <b>Scaling and Overflow Behavior:</b>
   * \par
   * The function is implemented using an internal 64-bit accumulator.
   * The accumulator has a 2.62 format and maintains full precision of the intermediate multiplication results but provides only a single guard bit.
   * Thus, if the accumulator result overflows it wraps around rather than clip.
   * In order to avoid overflows completely the input signal must be scaled down by 2 bits as there are four additions.
   * After all multiply-accumulates are performed, the 2.62 accumulator is truncated to 1.32 format and then saturated to 1.31 format.
   */
 q31_t arm_pid_q31__(
  arm_pid_instance_q31 * S,
  q31_t in)
  {
    q63_t acc;
    q31_t out;

    /* acc = A0 * x[n]  */
    acc = (q63_t) S->A0 * in;

    /* acc += A1 * x[n-1] */
    acc += (q63_t) S->A1 * S->state[0];

    /* acc += A2 * x[n-2]  */
    acc += (q63_t) S->A2 * S->state[1];

    /* convert output to 1.31 format to add y[n-1] */
   // out = (q31_t) (acc >> 31u);
    out = (q31_t) acc;

    /* out += y[n-1] */
    out += S->state[2];

    /* Update state */
    S->state[1] = S->state[0];
    S->state[0] = in;
    S->state[2] = out;

    /* return to application */
    return (out);
}

 q31_t arm_pid_q31(
  arm_pid_instance_q31 * S,
  q31_t in)
  {
	 q31_t acc;
    q31_t out;

    /* acc = A0 * x[n]  */
    acc =  S->A0 * in;

    /* acc += A1 * x[n-1] */
    acc +=  S->A1 * S->state[0];

    /* acc += A2 * x[n-2]  */
    acc +=  S->A2 * S->state[1];

    /* convert output to 1.31 format to add y[n-1] */
   // out = (q31_t) (acc >> 31u);
    out = (q31_t) acc;

    /* out += y[n-1] */
    out += S->state[2];

    /* Update state */
    S->state[1] = S->state[0];
    S->state[0] = in;
    S->state[2] = out;

    /* return to application */
    return (out);
}


 static __INLINE q31_t clip_q63_to_q31(
					q63_t x)
 {
   return ((q31_t) (x >> 32) != ((q31_t) x >> 31)) ?
     ((0x7FFFFFFF ^ ((q31_t) (x >> 62)))) : (q31_t) x;
 }

 static __INLINE q31_t __QADD(
			       q31_t x,
			       q31_t y)
 {
   return clip_q63_to_q31((q63_t) x + y);
 }


 void arm_pid_init_q31__(
   arm_pid_instance_q31 * S,
   int32_t resetStateFlag)
 {
   /* Derived coefficient A0 */
   S->A0 = __QADD(__QADD(S->Kp, S->Ki), S->Kd);

   /* Derived coefficient A1 */
   S->A1 = -__QADD(__QADD(S->Kd, S->Kd), S->Kp);

   /* Derived coefficient A2 */
   S->A2 = S->Kd;

   /* Check whether state needs reset or not */
   if(resetStateFlag)
   {
     /* Clear the state buffer.  The size will be always 3 samples */
     memset(S->state, 0, 3u * sizeof(q31_t));
   }

 }

 void arm_pid_init_q31(
   arm_pid_instance_q31 * S,
   int32_t resetStateFlag)
 {
	  /* Derived coefficient A0 */
	  S->A0 = S->Kp + S->Ki + S->Kd;

	  /* Derived coefficient A1 */
	  S->A1 = (-S->Kp) - ( 2 * S->Kd);

	  /* Derived coefficient A2 */
	  S->A2 = S->Kd;

	  /* Check whether state needs reset or not */
	  if(resetStateFlag)
	  {
	    /* Clear the state buffer.  The size will be always 3 samples */
	    memset(S->state, 0, 3u * sizeof(q31_t));
	  }

 }

 void arm_pid_reset_q31(
   arm_pid_instance_q31 * S)
 {

   /* Clear the state buffer.  The size will be always 3 samples */
   memset(S->state, 0, 3u * sizeof(q31_t));
 }









void arm_pid_reset_f32(
  arm_pid_instance_f32 * S)
{

  /* Clear the state buffer.  The size will be always 3 samples */
  memset(S->state, 0, 3u * sizeof(float32_t));
}


void arm_pid_init_f32(
  arm_pid_instance_f32 * S,
  int32_t resetStateFlag)
{

  /* Derived coefficient A0 */
  S->A0 = S->Kp + S->Ki + S->Kd;

  /* Derived coefficient A1 */
  S->A1 = (-S->Kp) - ((float32_t) 2.0 * S->Kd);

  /* Derived coefficient A2 */
  S->A2 = S->Kd;

  /* Check whether state needs reset or not */
  if(resetStateFlag)
  {
    /* Clear the state buffer.  The size will be always 3 samples */
    memset(S->state, 0, 3u * sizeof(float32_t));
  }

}



  /**
   * @brief  Process function for the floating-point PID Control.
   * @param[in,out] S   is an instance of the floating-point PID Control structure
   * @param[in]     in  input sample to process
   * @return out processed output sample.
   */
  float32_t arm_pid_f32(
  arm_pid_instance_f32 * S,
  float32_t in)
  {
    float32_t out;

    /* y[n] = y[n-1] + A0 * x[n] + A1 * x[n-1] + A2 * x[n-2]  */
    out = (S->A0 * in) +
      (S->A1 * S->state[0]) + (S->A2 * S->state[1]) + (S->state[2]);

    /* Update state */
    S->state[1] = S->state[0];
    S->state[0] = in;
    S->state[2] = out;

    /* return to application */
    return (out);

}
