// This file is part of COMPASS <https://github.com/COSMIC-RTC/compass>
//
// COMPASS is free software: you can redistribute it and/or modify it under the terms of the GNU
// Lesser General Public License as published by the Free Software Foundation, either version 3 of
// the License, or any later version.
//
// COMPASS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with COMPASS. If
// not, see <https://www.gnu.org/licenses/>
//
//  Copyright (C) 2011-2024 COSMIC Team <https://github.com/COSMIC-RTC/compass>

//! \file      sutra_controller_generic.cpp
//! \ingroup   libsutra
//! \class     SutraControllerGeneric
//! \brief     this class provides the controller_generic features to COMPASS
//! \author    COSMIC Team <https://github.com/COSMIC-RTC/compass>
//! \date      2022/01/24

#include <sutra_controller_generic.hpp>

template <typename T, typename Tout>
SutraControllerGeneric<T, Tout>::SutraControllerGeneric(
    CarmaContext *context, int64_t nslope, int64_t nactu, float delay,
    SutraDms *dms, int32_t *idx_dms, int32_t ndm, int32_t *idx_centro, int32_t ncentro, int32_t nstates)
    : SutraController<T, Tout>(context, nslope, nactu, delay, dms,
                                idx_dms, ndm, idx_centro, ncentro) {
  this->command_law = "integrator";
  this->nstates = nstates;
  this->leaky_factor = 1.0f;

  int64_t dims_data1[2] = {1, nactu + nstates};
  this->d_gain = new CarmaObj<T>(this->current_context, dims_data1);
  this->d_decayFactor = new CarmaObj<T>(this->current_context, dims_data1);
  this->d_compbuff = new CarmaObj<T>(this->current_context, dims_data1);
  for (int32_t k=0; k < this->d_circular_coms.size() ; k++) {
    delete this->d_circular_coms[k];
  }
  this->d_circular_coms.clear();
  this->d_com = new CarmaObj<T>(this->current_context, dims_data1);
  this->d_circular_coms.push_front(this->d_com);
  this->d_com1 = new CarmaObj<T>(this->current_context, dims_data1);
  if (this->delay > 0) {
      this->d_circular_coms.push_front(this->d_com1);
      while (this->d_circular_coms.size() <= int32_t(this->delay) + 1) {
        this->d_circular_coms.push_front(new CarmaObj<T>(context, dims_data1));
      }
    }

  for (int32_t cpt = 0; cpt < this->current_context->get_ndevice(); cpt++) {
    if( !this->current_context->can_p2p(this->device, cpt) ) {
      fprintf(stderr, "[%s@%d]: WARNING P2P not activate between %d and %d, it may failed\n",
        __FILE__, __LINE__, this->device, cpt);
    }
    this->P2Pdevices.push_back(cpt);
  }
  cudaEvent_t ev[this->P2Pdevices.size()];
  cudaStream_t st[this->P2Pdevices.size()];
  for (auto dev_id : this->P2Pdevices) {
    this->current_context->set_active_device(dev_id, 1);
    cudaEventCreateWithFlags(&ev[dev_id], cudaEventDisableTiming);
    this->events.push_back(ev[dev_id]);
    this->d_err_ngpu.push_back(new CarmaObj<T>(this->current_context, dims_data1));
    if (dev_id != this->device) {
      cudaStreamCreate(&st[dev_id]);
      this->streams.push_back(st[dev_id]);
    }
    else {
      this->streams.push_back(this->mainStream);
    }
  }
  this->current_context->set_active_device(this->device, 1);
  cudaEventCreateWithFlags(&start_mvm_event, cudaEventDisableTiming);
  dims_data1[1] = this->nslope();
  this->d_olmeas = new CarmaObj<T>(this->current_context, dims_data1);
  this->d_compbuff2 = new CarmaObj<T>(this->current_context, dims_data1);
  int64_t dims_data2[3] = {2, nactu + nstates, nslope};
  this->d_cmat = new CarmaObj<T>(this->current_context, dims_data2);
  this->d_cmat_ngpu.push_back(this->d_cmat);
  this->d_centroids_ngpu.push_back(this->d_centroids);
  if(this->P2Pdevices.size() > 1) {
    dims_data2[2] = this->nslope() / this->P2Pdevices.size();
    dims_data1[1] = this->nslope() / this->P2Pdevices.size();
    for (auto dev_id : this->P2Pdevices) {
      if(dev_id != this->P2Pdevices.back() && dev_id != this->device) {
        this->current_context->set_active_device(dev_id, 1);
        this->d_cmat_ngpu.push_back(new CarmaObj<T>(this->current_context, dims_data2));
        this->d_centroids_ngpu.push_back(new CarmaObj<T>(this->current_context, dims_data1));
      }
    }
    int32_t dev_id = this->P2Pdevices.back();
    int32_t cpt = this->P2Pdevices.size() - 1;
    dims_data2[2] = this->nslope() - cpt * (this->nslope() / this->P2Pdevices.size());
    dims_data1[1] = this->nslope() - cpt * (this->nslope() / this->P2Pdevices.size());
    this->current_context->set_active_device(dev_id, 1);
    this->d_cmat_ngpu.push_back(new CarmaObj<T>(this->current_context, dims_data2));
    this->d_centroids_ngpu.push_back(new CarmaObj<T>(this->current_context, dims_data1));

    this->current_context->set_active_device(this->device, 1);
  }
  this->gain = 0.f;

  dims_data2[2] = nactu + nstates;
  this->d_matE = new CarmaObj<T>(this->current_context, dims_data2);
  dims_data2[1] = this->nslope();
  this->d_imat = new CarmaObj<T>(this->current_context, dims_data2);

  this->polc = false;
}

template <typename T, typename Tout>
SutraControllerGeneric<T, Tout>::~SutraControllerGeneric() {
  this->current_context->set_active_device(this->device, 1);
  delete this->d_gain;
  delete this->d_decayFactor;
  delete this->d_cmat;
  delete this->d_matE;
  delete this->d_compbuff;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::set_polc(bool p) {
  this->polc = p;
  return EXIT_SUCCESS;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::set_leaky_factor(T factor) {
  this->leaky_factor = factor;
  return EXIT_SUCCESS;
}

template <typename T, typename Tout>
string SutraControllerGeneric<T, Tout>::get_type() {
  this->current_context->set_active_device(this->device, 1);
  return "generic";
}

template <typename T, typename Tout>
string SutraControllerGeneric<T, Tout>::get_commandlaw() {
  this->current_context->set_active_device(this->device, 1);
  return this->command_law;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::set_modal_gains(float *gain) {
  this->current_context->set_active_device(this->device, 1);
  this->d_gain->host2device(gain);
  return EXIT_SUCCESS;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::set_decayFactor(float *decayFactor) {
  this->current_context->set_active_device(this->device, 1);
  this->d_decayFactor->host2device(decayFactor);
  return EXIT_SUCCESS;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::set_matE(float *matE) {
  this->current_context->set_active_device(this->device, 1);
  this->d_matE->host2device(matE);
  return EXIT_SUCCESS;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::set_imat(float *imat) {
  this->current_context->set_active_device(this->device, 1);
  this->d_imat->host2device(imat);
  return EXIT_SUCCESS;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::set_cmat(float *cmat) {
  this->current_context->set_active_device(this->device, 1);
  // Copy the cmat on the master GPU and zero-fill it if needed
  this->d_cmat->host2device(cmat);
  // Distribute it on the available GPUs
  if(this->P2Pdevices.size() > 1) {
    this->distribute_cmat();
  }

  return EXIT_SUCCESS;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::distribute_cmat() {
    int32_t N = this->nactu() * (this->nslope() / this->P2Pdevices.size());
    int32_t cpt = 1;
    for (auto dev_id : this->P2Pdevices) {
      if(dev_id != this->device) {
        this->current_context->set_active_device(dev_id, 1);
        this->d_cmat_ngpu[cpt]->copy_from(this->d_cmat->get_data_at(cpt * N), this->d_cmat_ngpu[cpt]->get_nb_elements());
        cpt++;
      }
    }
    this->current_context->set_active_device(this->device, 1);

  return EXIT_SUCCESS;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::set_commandlaw(string law) {
  this->current_context->set_active_device(this->device, 1);
  this->command_law = law;
  return EXIT_SUCCESS;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::comp_polc(){
  comp_polc(*(this->d_centroids), *(this->d_imat), *(this->d_olmeas));
  return EXIT_SUCCESS;
}

template <typename T, typename Tout>
int32_t SutraControllerGeneric<T, Tout>::comp_com() {
  this->current_context->set_active_device(this->device, 1);
  CarmaObj<T> *centroids;
  T berta = this->leaky_factor;
  // cudaEvent_t start_event, stop_event;
  // carma_safe_call(cudaEventCreate(&start_event));
  // carma_safe_call(cudaEventCreate(&stop_event));
  // float gpuTime;
  int32_t m, n;

  if (this->polc) {
    this->comp_polc();
    centroids = this->d_olmeas;
    berta = T(1.0f - this->gain) * this->leaky_factor;
  } else {
    centroids = this->d_centroids;
  }
  int32_t cc = this->nslope() / this->P2Pdevices.size();
  cudaEventRecord(start_mvm_event, this->streams[this->device]);
  for (auto dev_id : this->P2Pdevices) {
    if(dev_id != this->device) {
      this->current_context->set_active_device(dev_id, 1);
      cudaStreamWaitEvent(this->streams[dev_id], start_mvm_event, 0);
      cudaMemcpyAsync(d_centroids_ngpu[dev_id]->get_data(),
                    centroids->get_data() + cc,
                    d_centroids_ngpu[dev_id]->get_nb_elements() * sizeof(T),
                    cudaMemcpyDeviceToDevice,
                    this->streams[dev_id]);
      cc += d_centroids_ngpu[dev_id]->get_nb_elements();
    }
  }
  this->current_context->set_active_device(this->device, 1);
  CarmaObj<T> *cmat;
    cmat = this->d_cmat;
    m = this->d_cmat->get_dims(1);
    n = this->d_cmat->get_dims(2);

  if (this->command_law == "integrator") {
    // cublasSetStream(this->cublas_handle(),
    //                 current_context->get_device(device)->get_stream());
    // carma_safe_call(cudaEventRecord(start_event));
    if(this->P2Pdevices.size() > 1) {
      // cudaEventRecord(start_mvm_event, this->streams[this->device]);
      for (auto dev_id : this->P2Pdevices) {
          this->current_context->set_active_device(dev_id, 1);
          if(dev_id != this->device) {
            n = this->d_cmat_ngpu[dev_id]->get_dims()[2];
          }
          else {
            n = this->nslope() / this->P2Pdevices.size();
          }
          cublasSetStream(this->cublas_handle(), this->streams[dev_id]);
          // cudaStreamWaitEvent(this->streams[dev_id], start_mvm_event, 0); // make sure mainstream is ready before launching sub mvm
          carma_gemv(this->cublas_handle(), 'n', m, n, (T)(-1 * this->gain),
                this->d_cmat_ngpu[dev_id]->get_data(), m, d_centroids_ngpu[dev_id]->get_data(), 1, T(0.f),
                this->d_err_ngpu[dev_id]->get_data(), 1);
          cudaEventRecord(this->events[dev_id], this->streams[dev_id]);
      }
      // Finally, we reduce all the results
      this->current_context->set_active_device(this->device, 1);
      cublasSetStream(this->cublas_handle(), this->streams[this->device]);
      for (auto dev_id : this->P2Pdevices) {
          cudaStreamWaitEvent(this->streams[this->device], this->events[dev_id], 0);
          this->d_com->axpy(berta, this->d_err_ngpu[dev_id], 1,1);
      }
    // carma_safe_call(cudaEventRecord(stop_event));
    // carma_safe_call(cudaEventSynchronize(stop_event));
    // carma_safe_call(cudaEventElapsedTime(&gpuTime, start_event, stop_event));
    // DEBUG_TRACE("GEMV DONE IN %f ms", gpuTime);

    } else { // Single GPU case or P2P not available between all GPUs
      cublasSetStream(this->cublas_handle(), this->mainStream);
      carma_gemv(this->cublas_handle(), 'n', m, n, (T)(-1 * this->gain),
               cmat->get_data(), m, centroids->get_data(), 1, berta,
               this->d_com->get_data(), 1);
    }
  } else {
    // CMAT*s(k)
    carma_gemv(this->cublas_handle(), 'n', this->nactu() + this->nstates, this->nslope(),
               (T)1.0f, this->d_cmat->get_data(), this->nactu() + this->nstates,
               centroids->get_data(), 1, (T)0.0f, this->d_compbuff->get_data(),
               1);
    // g*CMAT*s(k)
    mult_vect(this->d_compbuff->get_data(), this->d_gain->get_data(), (T)(-1.0f),
              this->nactu() + this->nstates, this->current_context->get_device(this->device));
    if (this->command_law == "modal_integrator") {
      // M2V * g * CMAT * s(k)
      carma_gemv(this->cublas_handle(), 'n', this->nactu() + this->nstates, this->nactu() + this->nstates,
                 this->gain, this->d_matE->get_data(), this->nactu() + this->nstates,
                 this->d_compbuff->get_data(), 1, berta, this->d_com->get_data(),
                 1);
    } else {  // 2matrices
      if(!this->delay)
        this->d_com1->copy_from(this->d_com->get_data(), this->d_com->get_nb_elements());
      carma_gemv(this->cublas_handle(), 'n', this->nactu() + this->nstates, this->nactu() + this->nstates,
                 (T)1.0f, this->d_matE->get_data(), this->nactu() + this->nstates,
                 this->d_com1->get_data(), 1, (T)0.0f, this->d_com->get_data(),
                 1);
      // v(k) = alpha*E*v(k-1)
      mult_vect(this->d_com->get_data(), this->d_decayFactor->get_data(), (T)1.0f,
                this->nactu() + this->nstates, this->current_context->get_device(this->device));
      // v(k) = alpha*E*v(k-1) + g*CMAT*s(k)
      this->d_com->axpy((T)1.0f, this->d_compbuff, 1, 1);
    }
  }
  cublasSetStream(this->cublas_handle(), this->mainStream);
  //   cudaEventDestroy(start_event);
  // cudaEventDestroy(stop_event);
  return EXIT_SUCCESS;
}

template class SutraControllerGeneric<float, float>;
template class SutraControllerGeneric<float, uint16_t>;
