// -----------------------------------------------------------------------------
//  This file is part of COMPASS <https://anr-compass.github.io/compass/>
//
//  Copyright (C) 2011-2019 COMPASS Team <https://github.com/ANR-COMPASS>
//  All rights reserved.
//  Distributed under GNU - LGPL
//
//  COMPASS is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser
//  General Public License as published by the Free Software Foundation, either version 3 of the License,
//  or any later version.
//
//  COMPASS: End-to-end AO simulation tool using GPU acceleration
//  The COMPASS platform was designed to meet the need of high-performance for the simulation of AO systems.
//
//  The final product includes a software package for simulating all the critical subcomponents of AO,
//  particularly in the context of the ELT and a real-time core based on several control approaches,
//  with performances consistent with its integration into an instrument. Taking advantage of the specific
//  hardware architecture of the GPU, the COMPASS tool allows to achieve adequate execution speeds to
//  conduct large simulation campaigns called to the ELT.
//
//  The COMPASS platform can be used to carry a wide variety of simulations to both testspecific components
//  of AO of the E-ELT (such as wavefront analysis device with a pyramid or elongated Laser star), and
//  various systems configurations such as multi-conjugate AO.
//
//  COMPASS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
//  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License along with COMPASS.
//  If not, see <https://www.gnu.org/licenses/lgpl-3.0.txt>.
// -----------------------------------------------------------------------------

//! \file      sutra_controller_generic_linear.h
//! \ingroup   libsutra
//! \class     sutra_controller_generic_linear
//! \brief     this class provides the controller_generic features to COMPASS
//! \author    COMPASS Team <https://github.com/ANR-COMPASS>
//! \version   5.2.0
//! \date      2011/01/28
//! \copyright GNU Lesser General Public License

#ifndef _SUTRA_CONTROLLER_GENERIC_LINEAR_H_
#define _SUTRA_CONTROLLER_GENERIC_LINEAR_H_

#include <sutra_acquisim.h>
#include <sutra_controller.h>

#include <memory>

template <typename Tcomp, typename Tout>
class sutra_controller_generic_linear : public SutraController<Tcomp, Tout> {


private:
  using SutraController<Tcomp,Tout>::nslopes; //!< num of meas in slope vector
  using SutraController<Tcomp,Tout>::nactus;  //!< num of coms in com vector

  bool m_polc;              //!< flag to compute POL slopes
  bool m_modal;             //!< flag to do projection from modes to actu
  int m_n_slope_buffers;    //!< num of historic slopes vectors to use
  int m_n_states;           //!< num of states in state vector
  int m_n_state_buffers;    //!< num of historic state vectors to use
  int m_n_modes;            //!< num of modes in mode vector
  int m_n_iir_in;           //!< num of input mode vectors for iir filter
  int m_n_iir_out;          //!< num of output mode vectors for iir filter

  using SutraController<Tcomp,Tout>::a; //!< coefficient used to comp delay
  using SutraController<Tcomp,Tout>::b; //!< coefficient used to comp delay

  using SutraController<Tcomp,Tout>::cublas_handle;

public:

  bool polc(){return m_polc;}
  bool modal(){return m_modal;}
  int n_slope_buffers(){return m_n_slope_buffers;}
  int n_states(){return m_n_states;}
  int n_state_buffers(){return m_n_state_buffers;}
  int n_modes(){return m_n_modes;}
  int n_iir_in(){return m_n_iir_in;}
  int n_iir_out(){return m_n_iir_out;}

  std::unique_ptr<CarmaObj<Tcomp>> d_x_now;   //!< vector used for state calcs
  std::unique_ptr<CarmaObj<Tcomp>> d_s_now;   //!< vector used for slope calcs
  std::unique_ptr<CarmaObj<Tcomp>> d_u_now;   //!< vector used for modal calcs

  std::deque<CarmaObj<Tcomp> *> d_circular_x;     //!< circ buffer states
  std::deque<CarmaObj<Tcomp> *> d_circular_s;     //!< circ buffer slopes
  std::deque<CarmaObj<Tcomp> *> d_circular_u_in;  //!< circ buffer iir inputs
  std::deque<CarmaObj<Tcomp> *> d_circular_u_out; //!< circ buffer iir output

  std::vector<CarmaObj<Tcomp>*> d_matA;      //!< list of A matrices (recursions)
  std::vector<CarmaObj<Tcomp>*> d_matL;      //!< list of L matrices (innovations)
  std::unique_ptr<CarmaObj<Tcomp>> d_matK;   //!< K matrix (state to modes)
  std::unique_ptr<CarmaObj<Tcomp>> d_matD;   //!< D matrix (interaction matrix)
  std::unique_ptr<CarmaObj<Tcomp>> d_matF;   //!< F matrix (mode to actu)

  std::vector<CarmaObj<Tcomp>*> d_iir_a;  //!< list of iir 'a' vectors (outputs)
  std::vector<CarmaObj<Tcomp>*> d_iir_b;  //!< list of iir 'b' vectors (inputs)

  using SutraController<Tcomp,Tout>::d_com;  //!< most recently computed command
  using SutraController<Tcomp,Tout>::d_centroids;  //!< closed loop slope vector
  using SutraController<Tcomp,Tout>::d_circular_coms; //!< circular com buffer
  using SutraController<Tcomp,Tout>::delay;

 public:
 sutra_controller_generic_linear() = delete;
  sutra_controller_generic_linear(
    CarmaContext *context, int nslope, int nslopes_buffers, int nactu, int nstates,
    int nstates_buffers, int nmodes, int niir_in, int niir_out,
    float delay, bool polc, bool is_modal,
    SutraDms *dms, int *idx_dms, int ndm, int *idx_centro, int ncentro);
  //sutra_controller_generic_linear(const sutra_controller_generic_linear &controller);
  ~sutra_controller_generic_linear();

  string get_type();
  string get_commandlaw();
  int set_matA(float * A, int i);
  int set_matL(float * L, int i);
  int set_matK(float * K);
  int set_matD(float * D);
  int set_matF(float * F);
  int set_iir_a(float * iir_a, int i);
  int set_iir_b(float * iir_b, int i);
  int set_polc(bool p);
  using SutraController<Tcomp,Tout>::comp_polc;
  int comp_polc();
  int comp_com();

  int recursion();
  int innovation();
  int modal_projection();
  int filter_iir_in();
  int filter_iir_out();

};

template <typename T>
void pad_cmat(T *idata, int m, int n, T *odata, int m2, int n2,
              CarmaDevice *device);
#endif  // _SUTRA_CONTROLLER_GENERIC_LINEAR_H_