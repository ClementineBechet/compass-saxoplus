//kp_kalman_core.h
//Kalman controller (core)


#ifndef __LAM__KP_KALMAN_CORE_FULL__H__
#define __LAM__KP_KALMAN_CORE_FULL__H__

#include "kp_matrix.h"
#include <string>


class kp_kalman_core_full
{
 public:

   //make initialization and calculate gain
   kp_kalman_core_full(const kp_matrix& D_Mo_,
		  const kp_matrix& N_Act_,
		  const kp_matrix& PROJ_,
		  bool isZonal_);

   
   virtual void calculate_gain(real bruit,
			       real k_W,
			       const  kp_matrix&  SigmaV,
			       const  kp_vector& atur_,
			       const  kp_vector& btur_) = 0;
   
   
   virtual void next_step(const kp_vector& Y_k, kp_vector& U_k) = 0;

   virtual ~kp_kalman_core_full(){}

 protected:
   //kp_matrix D_Mo, N_Act, PROJ;
   //kp_vector atur;
   //kp_vector btur;
   int ordreAR;
   int nb_p, nb_act, nb_z, nb_az, nb_n;
   bool gainComputed;
   bool variablesInitialized;
   bool isZonal;
};


#endif