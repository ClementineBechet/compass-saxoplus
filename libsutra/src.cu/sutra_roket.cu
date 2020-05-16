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

//! \file      sutra_roket.cu
//! \ingroup   libsutra
//! \class     SutraRoket
//! \brief     this class provides the roket features to COMPASS
//! \author    COMPASS Team <https://github.com/ANR-COMPASS>
//! \version   5.0.0
//! \date      2011/01/28
//! \copyright GNU Lesser General Public License

#include <sutra_roket.h>

__global__ void separate_modes_krnl(float *modes, float *filtmodes, int nmodes,
                                    int nfilt) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  int bornemin = nmodes - nfilt - 2;
  int bornemax = nmodes - 2;
  while (tid < nmodes) {
    if (tid >= bornemin && tid < bornemax) {
      filtmodes[tid] = modes[tid];
      modes[tid] = 0.0f;
    } else
      filtmodes[tid] = 0.0f;

    tid += blockDim.x * gridDim.x;
  }
}

int separate_modes(float *modes, float *filtmodes, int nmodes, int nfilt,
                   CarmaDevice *device) {
  int nb_threads = 0, nb_blocks = 0;
  get_num_blocks_and_threads(device, nmodes, nb_blocks, nb_threads);
  dim3 grid(nb_blocks), threads(nb_threads);

  separate_modes_krnl<<<grid, threads>>>(modes, filtmodes, nmodes, nfilt);
  carma_check_msg("separate_modes_krnl<<<>>> execution failed\n");

  return EXIT_SUCCESS;
}
