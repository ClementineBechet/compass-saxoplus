#include <sutra_roket.h>

sutra_roket::sutra_roket(carma_context *context, int device, sutra_rtc *rtc, sutra_sensors *sensors,
                sutra_target *target, sutra_dms *dms, sutra_telescope *tel, sutra_atmos *atm, int loopcontroller, int geocontroller,
                int nactus, int nmodes, int nfilt, int niter, float *Btt, float *P, float *gRD, float *RD){

    // context
    this->current_context = context;
    this->device = device;
    this->current_context->set_activeDevice(device,1);
    // sutra objects to supervise
    this->rtc = rtc;
    this->sensors = sensors;
    this->target = target;
    this->dms = dms;
    this->atm = atm;
    this->tel = tel;
    this->loopcontrol = 0L;
    this->geocontrol = 0L;
    // simple initialisation
    this->niter = niter;
    this->nfilt = nfilt;
    this->nactus = nactus;
    this->nmodes = nmodes;
    this->iterk = 0;
    this->loopcontroller = loopcontroller;
    this->geocontroller = geocontroller;
    if (this->rtc->d_control[this->loopcontroller]->get_type().compare("ls") == 0) {
      this->loopcontrol = dynamic_cast<sutra_controller_ls *>(this->rtc->d_control[this->loopcontroller]);
    }
    if (this->rtc->d_control[this->geocontroller]->get_type().compare("geo") == 0) {
      this->geocontrol = dynamic_cast<sutra_controller_geo *>(this->rtc->d_control[this->geocontroller]);
    }
    this->gain = loopcontrol->gain;
    this->fitting = 0.;

    // contributors buffers initialsations
    long dims_data2[3] = {2,this->niter,this->nactus};
    this->d_noise = new carma_obj<float>(this->current_context, dims_data2);
    this->d_nonlinear = new carma_obj<float>(this->current_context, dims_data2);
    this->d_tomo = new carma_obj<float>(this->current_context, dims_data2);
    this->d_filtered = new carma_obj<float>(this->current_context, dims_data2);
    this->d_alias = new carma_obj<float>(this->current_context, dims_data2);
    this->d_bandwidth = new carma_obj<float>(this->current_context, dims_data2);
    this->d_commanded = new carma_obj<float>(this->current_context, dims_data2);

    // Matrix initialisation
    dims_data2[1] = this->nmodes;
    this->d_P = new carma_obj<float>(this->current_context,dims_data2,P);
    dims_data2[1] = this->nactus;
    this->d_gRD = new carma_obj<float>(this->current_context,dims_data2,gRD);
    this->d_RD = new carma_obj<float>(this->current_context,dims_data2,RD);
    dims_data2[2] = this->nmodes;
    this->d_Btt = new carma_obj<float>(this->current_context,dims_data2,P);

    // Residual error buffer initialsations
    long dims_data1[2] = {1,this->nactus};
    this->d_fullErr = new carma_obj<float>(this->current_context,dims_data1);
    this->d_err1 = new carma_obj<float>(this->current_context,dims_data1);
    this->d_err2 = new carma_obj<float>(this->current_context,dims_data1);
    this->d_bkup_com = new carma_obj<float>(this->current_context,dims_data1);
    this->d_tmpdiff = new carma_obj<float>(this->current_context,dims_data1);

    // Additional buffers initialsations
    dims_data1[1] = this->nmodes;
    this->d_modes = new carma_obj<float>(this->current_context,dims_data1);
    this->d_filtmodes = new carma_obj<float>(this->current_context,dims_data1);

    // Target screen backup
    this->d_bkup_screen = new carma_obj<float>(this->current_context, this->target->d_targets[0]->d_phase->d_screen->getDims());

}

sutra_roket::~sutra_roket(){
    this->current_context->set_activeDevice(this->device,1);
    if(this->d_P)
        delete this->d_P;
    if(this->d_Btt)
        delete this->d_Btt;
    if(this->d_noise)
        delete this->d_noise;
    if(this->d_nonlinear)
        delete this->d_nonlinear;
    if(this->d_tomo)
        delete this->d_tomo;
    if(this->d_filtered)
        delete this->d_filtered;
    if(this->d_alias)
        delete this->d_alias;
    if(this->d_bandwidth)
        delete this->d_bandwidth;
    if(this->d_fullErr)
        delete this->d_fullErr;
    if(this->d_err1)
        delete this->d_err1;
    if(this->d_err2)
        delete this->d_err2;
    if(this->d_bkup_screen)
        delete this->d_bkup_screen;
    if(this->d_bkup_com)
        delete this->d_bkup_com;
    if(this->d_commanded)
        delete this->d_commanded;
    if(this->d_modes)
        delete this->d_modes;
    if(this->d_filtmodes)
        delete this->d_filtmodes;
    if(this->d_gRD)
        delete this->d_gRD;
    if(this->d_RD)
        delete this->d_RD;
    if(this->d_tmpdiff)
        delete this->d_tmpdiff;
}

int sutra_roket::save_loop_state(){
  this->current_context->set_activeDevice(this->device,1);
  this->d_fullErr->copyFrom(this->loopcontrol->d_err->getData(),this->nactus);
  this->d_bkup_com->copyFrom(this->loopcontrol->d_com->getData(),this->nactus);
  this->d_bkup_screen->copyFrom(this->target->d_targets[0]->d_phase->d_screen->getData(),
                                this->target->d_targets[0]->d_phase->d_screen->getNbElem());

  return EXIT_SUCCESS;
}

int sutra_roket::restore_loop_state(){
  this->current_context->set_activeDevice(this->device,1);
  this->d_bkup_com->copyInto(this->loopcontrol->d_com->getData(),this->nactus);
  this->d_bkup_screen->copyInto(this->target->d_targets[0]->d_phase->d_screen->getData(),
                                this->target->d_targets[0]->d_phase->d_screen->getNbElem());

  return EXIT_SUCCESS;
}

int sutra_roket::apply_loop_filter(carma_obj<float> *d_odata, carma_obj<float> *d_idata1,
                                    carma_obj<float> *d_idata2, float gain, int k){
  this->current_context->set_activeDevice(this->device,1);
  this->d_tmpdiff->copyFrom(d_idata1->getData(),this->nactus);
  this->d_tmpdiff->axpy(-1.0f,d_idata2,1,1); // err1-err2
  this->d_tmpdiff->copyInto(d_odata->getData((k+1)*this->nactus),this->nactus); //odata[k+1,:] = err1 - err2
  carma_gemv<float>(this->current_context->get_cublasHandle(), 'n', this->nactus,
                    this->nactus, 1.0f, this->d_gRD->getData(),this->nactus,
                    d_odata->getData(k*this->nactus) , 1,
                    gain, d_odata->getData((k+1)*this->nactus), 1); // odata[k+1,:] = gRD*odata[k,:] + g*(err1-err2)

  return EXIT_SUCCESS;
}

int sutra_roket::compute_breakdown(){
  this->current_context->set_activeDevice(this->device,1);
  save_loop_state();
  // Noise
  this->rtc->do_centroids(this->loopcontroller,false);
  this->rtc->do_control(this->loopcontroller);
  this->d_err1->copyFrom(this->loopcontrol->d_err->getData(),this->nactus);
  apply_loop_filter(this->d_noise,this->d_fullErr,this->d_err1,this->gain,this->iterk);
  // Non-linearity
  this->rtc->do_centroids_geom(this->loopcontroller);
  this->rtc->do_control(this->loopcontroller);
  this->d_err2->copyFrom(this->loopcontrol->d_err->getData(),this->nactus);
  apply_loop_filter(this->d_nonlinear,this->d_err1,this->d_err2,this->gain,this->iterk);
  // Aliasing on GS direction
  this->geocontrol->comp_dphi(this->sensors->d_wfs[0]->d_gs,true);
  this->rtc->do_control(this->geocontroller);
  this->rtc->apply_control(this->geocontroller,this->dms);
  this->sensors->d_wfs[0]->sensor_trace(this->dms, 0);
  this->sensors->d_wfs[0]->d_gs->d_phase->d_screen->axpy(1.0, this->tel->d_phase_ab_M1_m, 1, 1);
  this->sensors->d_wfs[0]->comp_image();
  this->rtc->do_centroids(this->loopcontroller,false);
  this->rtc->do_control(this->loopcontroller);
  this->d_err1->copyFrom(this->loopcontrol->d_err->getData(),this->nactus);
  this->d_err2->copyFrom(this->d_tmpdiff->getData(),this->nactus);
  apply_loop_filter(this->d_alias,this->d_err1,this->d_err2,this->gain,this->iterk);
  // Wavefront
  this->target->d_targets[0]->raytrace(this->atm);
  this->target->d_targets[0]->d_phase->d_screen->axpy(1.0, this->tel->d_phase_ab_M1_m, 1, 1);
  this->current_context->set_activeDevice(this->rtc->device,1);
  this->geocontrol->comp_dphi(this->target->d_targets[0],false);

  this->rtc->do_control(this->geocontroller);
  this->d_err1->copyFrom(this->geocontrol->d_com->getData(),this->nactus);
  //Fitting
  this->rtc->apply_control(this->geocontroller,this->dms);

  this->target->d_targets[0]->raytrace(this->dms,0,0);
  this->fitting += this->target->d_targets[0]->phase_var / this->niter;
  // Filtered modes
  carma_gemv<float>(this->current_context->get_cublasHandle(), 'n', this->nmodes,
                    this->nactus, 1.0f, this->d_P->getData(),this->nmodes,
                    this->d_err1->getData() , 1,
                    0.f, this->d_modes->getData(), 1);
  separate_modes(this->d_modes->getData(),this->d_filtmodes->getData(),this->nmodes,this->nfilt,this->current_context->get_device(this->device));
  carma_gemv<float>(this->current_context->get_cublasHandle(), 'n', this->nactus,
                    this->nmodes, 1.0f, this->d_Btt->getData(),this->nactus,
                    this->d_filtmodes->getData() , 1,
                    0.f, this->d_filtered->getData(this->iterk*this->nactus), 1);
  // Commanded modes
  carma_gemv<float>(this->current_context->get_cublasHandle(), 'n', this->nactus,
                    this->nmodes, 1.0f, this->d_Btt->getData(),this->nactus,
                    this->d_modes->getData() , 1,
                    0.f, this->d_commanded->getData(this->iterk*this->nactus), 1);
  // Bandwidth
  if(this->iterk > 0){
    this->d_err1->copyFrom(this->d_commanded->getData(this->iterk*this->nactus),this->nactus);
    this->d_err1->copyFrom(this->d_commanded->getData((this->iterk-1)*this->nactus),this->nactus);
    apply_loop_filter(this->d_bandwidth,this->d_err1,this->d_err2,1.0f,this->iterk-1);
  }
  // tomography
  this->sensors->d_wfs[0]->sensor_trace(this->atm);
  this->geocontrol->comp_dphi(this->sensors->d_wfs[0]->d_gs,true);
  this->rtc->do_control(this->geocontroller);
  this->d_err1->copyFrom(this->geocontrol->d_com->getData(),this->nactus);
  carma_gemv<float>(this->current_context->get_cublasHandle(), 'n', this->nmodes,
                    this->nactus, 1.0f, this->d_P->getData(),this->nmodes,
                    this->d_err1->getData() , 1,
                    0.f, this->d_modes->getData(), 1);
  separate_modes(this->d_modes->getData(),this->d_filtmodes->getData(),this->nmodes,this->nfilt,this->current_context->get_device(this->device));
  carma_gemv<float>(this->current_context->get_cublasHandle(), 'n', this->nactus,
                    this->nmodes, 1.0f, this->d_Btt->getData(),this->nactus,
                    this->d_modes->getData() , 1,
                    0.f, this->d_err2->getData(), 1);
  this->d_err1->copyFrom(this->d_commanded->getData(this->iterk*this->nactus),this->nactus);
  this->d_err1->axpy(-1.0f,this->d_err2,1,1);
  carma_gemv<float>(this->current_context->get_cublasHandle(), 'n', this->nactus,
                    this->nactus, 1.0f, this->d_RD->getData(),this->nactus,
                    this->d_err1->getData() , 1,
                    0.f, this->d_err2->getData(), 1);
  carmaSafeCall(cudaMemset(this->d_err1, 0, sizeof(float) * this->nactus));
  apply_loop_filter(this->d_tomo,this->d_err2,this->d_err1,this->gain,this->iterk);

  restore_loop_state();
  this->iterk++;



  return EXIT_SUCCESS;
}
