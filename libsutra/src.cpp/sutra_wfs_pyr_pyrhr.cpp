#include <sutra_wfs_pyr_pyrhr.h>
#include <sutra_ao_utils.h>
#include <carma_utils.h>

sutra_wfs_pyr_pyrhr::sutra_wfs_pyr_pyrhr(carma_context *context,
                                         sutra_telescope *d_tel,
                                         sutra_sensors *sensors, long nxsub,
                                         long nvalid, long npix, long nphase,
                                         long nrebin, long nfft, long ntot,
                                         long npup, float pdiam, float nphotons,
                                         float nphot4imat, int lgs, int device) :
    sutra_wfs_pyr(context, d_tel, sensors, nxsub, nvalid, npix, nphase, nrebin,
                  nfft, ntot, npup, pdiam, nphotons, nphot4imat, lgs, device,
                  "pyrhr") {
}

sutra_wfs_pyr_pyrhr::sutra_wfs_pyr_pyrhr(carma_context *context,
                                         sutra_telescope *d_tel,
                                         sutra_sensors *sensors, long nxsub,
                                         long nvalid, long npix, long nphase,
                                         long nrebin, long nfft, long ntot,
                                         long npup, float pdiam, float nphotons,
                                         float nphot4imat, int lgs,
                                         int nbdevices, int* devices) :
    sutra_wfs_pyr(context, d_tel, sensors, nxsub, nvalid, npix, nphase, nrebin,
                  nfft, ntot, npup, pdiam, nphotons, nphot4imat, lgs,
                  devices[0], "pyrhr") {
  long dims_data2[3];
  dims_data2[0] = 2;

  d_hrimg_ngpu.push_back(this->d_hrimg);
  d_camplipup_ngpu.push_back(this->d_camplipup);
  d_camplifoc_ngpu.push_back(this->d_camplifoc);
  d_phalfxy_ngpu.push_back(this->d_phalfxy);
  d_fttotim_ngpu.push_back(this->d_fttotim);
  d_screen_ngpu.push_back(nullptr); // init in the sutra_wfs_pyr_pyrhr::wfs_initarrays
  d_pupil_ngpu.push_back(this->d_pupil);

  for (int device = 1; device < nbdevices/2; device++) {
    current_context->set_activeDevice(device,1);
    dims_data2[1] = nfft;
    dims_data2[2] = nfft;
    d_hrimg_ngpu.push_back(new carma_obj<float>(context, dims_data2)); // Useless for SH
    d_camplipup_ngpu.push_back(
        new carma_obj<cuFloatComplex>(context, dims_data2));
    d_camplifoc_ngpu.push_back(
        new carma_obj<cuFloatComplex>(context, dims_data2));
    cufftHandle *plan = this->d_camplipup_ngpu[device]->getPlan(); ///< FFT plan
    carmafftSafeCall(
        cufftPlan2d(plan, dims_data2[1], dims_data2[2], CUFFT_C2C));
    d_phalfxy_ngpu.push_back(
        new carma_obj<cuFloatComplex>(context, dims_data2));
    d_fttotim_ngpu.push_back(
        new carma_obj<cuFloatComplex>(context, dims_data2));

    dims_data2[1] = ntot;
    dims_data2[2] = ntot;
    d_pupil_ngpu.push_back(new carma_obj<float>(context, dims_data2));
    d_screen_ngpu.push_back(new carma_obj<float>(context, dims_data2));
  }
}

sutra_wfs_pyr_pyrhr::~sutra_wfs_pyr_pyrhr() {
  for (std::vector<carma_obj<cuFloatComplex> *>::iterator it =
      this->d_camplipup_ngpu.begin(); this->d_camplipup_ngpu.end() != it;
      it++) {
    if (*it != this->d_camplipup) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      delete *it;
    }
  }
  this->d_camplipup_ngpu.clear();

  for (std::vector<carma_obj<cuFloatComplex> *>::iterator it =
      this->d_camplifoc_ngpu.begin(); this->d_camplifoc_ngpu.end() != it;
      it++) {
    if (*it != this->d_camplifoc) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      delete *it;
    }
  }
  this->d_camplifoc_ngpu.clear();

  for (std::vector<carma_obj<cuFloatComplex> *>::iterator it =
      this->d_phalfxy_ngpu.begin(); this->d_phalfxy_ngpu.end() != it; it++) {
    if (*it != this->d_phalfxy) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      delete *it;
    }
  }
  this->d_phalfxy_ngpu.clear();

  for (std::vector<carma_obj<cuFloatComplex> *>::iterator it =
      this->d_fttotim_ngpu.begin(); this->d_fttotim_ngpu.end() != it; it++) {
    if (*it != this->d_fttotim) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      delete *it;
    }
  }
  this->d_fttotim_ngpu.clear();

  for (std::vector<carma_obj<float> *>::iterator it =
      this->d_pupil_ngpu.begin(); this->d_pupil_ngpu.end() != it; it++) {
    if (*it != this->d_pupil) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      delete *it;
    }
  }
  this->d_pupil_ngpu.clear();

  for (std::vector<carma_obj<float> *>::iterator it =
      this->d_screen_ngpu.begin(); this->d_screen_ngpu.end() != it; it++) {
    if (*it != this->d_gs->d_phase->d_screen) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      delete *it;
    }
  }
  this->d_screen_ngpu.clear();

  for (std::vector<carma_obj<float> *>::iterator it =
      this->d_hrimg_ngpu.begin(); this->d_hrimg_ngpu.end() != it; it++) {
    if (*it != this->d_hrimg) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      delete *it;
    }
  }
  this->d_hrimg_ngpu.clear();

}

int sutra_wfs_pyr_pyrhr::wfs_initarrays(cuFloatComplex *halfxy, float *cx,
                                        float *cy, float *sincar,
                                        int *validsubsx, int *validsubsy,
                                        int *phasemap, float *fluxPerSub) {
  for (std::vector<carma_obj<cuFloatComplex> *>::iterator it =
      this->d_phalfxy_ngpu.begin(); this->d_phalfxy_ngpu.end() != it; it++) {
    if (*it != this->d_phalfxy) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      (*it)->host2device(halfxy);
    }
  }
  for (std::vector<carma_obj<float> *>::iterator it =
      this->d_pupil_ngpu.begin(); this->d_pupil_ngpu.end() != it; it++) {
    if (*it != this->d_pupil) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      (*it)->copyFrom(d_pupil->getData(), d_pupil->getNbElem());
    }
  }
  if(d_screen_ngpu.size()>0)
    d_screen_ngpu[0] = this->d_gs->d_phase->d_screen;
  current_context->set_activeDevice(device,1);
  this->d_phalfxy->host2device(halfxy);
  this->pyr_cx->fill_from(cx);
  this->pyr_cy->fill_from(cy);
  this->d_sincar->host2device(sincar);
  this->d_validsubsx->host2device(validsubsx);
  this->d_validsubsy->host2device(validsubsy);
  this->d_phasemap->host2device(phasemap);
  this->d_fluxPerSub->host2device(fluxPerSub);

  return EXIT_SUCCESS;
}

void sutra_wfs_pyr_pyrhr::comp_modulation(int cpt) {
  //int cpt = 0;
  int ngpu = d_screen_ngpu.size();
  if (ngpu < 2) {
    carmaSafeCall(
        cudaMemset(this->d_camplipup->getData(), 0,
                   2 * sizeof(float) * this->d_camplipup->getNbElem()));
    pyr_getpup(this->d_camplipup->getData(),
               this->d_gs->d_phase->d_screen->getData(),
               this->d_pupil->getData(), this->ntot, this->nfft,
               this->d_gs->lambda, (this->pyr_cx->getData())[cpt],
               (this->pyr_cy->getData())[cpt],
               this->current_context->get_device(device));
    carma_fft(this->d_camplipup->getData(), this->d_camplifoc->getData(), -1,
              *this->d_camplipup->getPlan());
    /*
     pyr_submask(this->d_camplifoc->getData(), this->d_submask->getData(),
     this->nfft, this->current_context->get_device(device));
     */
    pyr_submaskpyr(this->d_camplifoc->getData(), this->d_phalfxy->getData(),
                   this->nfft, this->current_context->get_device(device));
    carma_fft(this->d_camplifoc->getData(), this->d_fttotim->getData(), 1,
              *this->d_camplipup->getPlan());

    //float fact = 1.0f / this->nfft / this->nfft / this->nfft / 2.0;
    float fact = 1.0f;
    abs2(this->d_hrimg->getData(), this->d_fttotim->getData(),
         this->nfft * this->nfft, fact,
         this->current_context->get_device(device));
  } else {
    int cur_device = cpt % ngpu;
    current_context->set_activeDevice(cur_device,1);

    carmaSafeCall(
        cudaMemset(
            this->d_camplipup_ngpu[cur_device]->getData(),
            0,
            2 * sizeof(float)
            * this->d_camplipup_ngpu[cur_device]->getNbElem()));
    pyr_getpup(this->d_camplipup_ngpu[cur_device]->getData(),
               this->d_screen_ngpu[cur_device]->getData(),
               this->d_pupil_ngpu[cur_device]->getData(), this->ntot,
               this->nfft, this->d_gs->lambda, (this->pyr_cx->getData())[cpt],
               (this->pyr_cy->getData())[cpt],
               this->current_context->get_device(cur_device));
    carma_fft(this->d_camplipup_ngpu[cur_device]->getData(),
              this->d_camplifoc_ngpu[cur_device]->getData(), -1,
              *this->d_camplipup_ngpu[cur_device]->getPlan());
    /*
     pyr_submask(this->d_camplifoc->getData(), this->d_submask->getData(),
     this->nfft, this->current_context->get_device(device));
     */
    pyr_submaskpyr(this->d_camplifoc_ngpu[cur_device]->getData(),
                   this->d_phalfxy_ngpu[cur_device]->getData(), this->nfft,
                   this->current_context->get_device(cur_device));
    carma_fft(this->d_camplifoc_ngpu[cur_device]->getData(),
              this->d_fttotim_ngpu[cur_device]->getData(), 1,
              *this->d_camplipup_ngpu[cur_device]->getPlan());
    //float fact = 1.0f / this->nfft / this->nfft / this->nfft / 2.0;
    float fact = 1.0f;
    abs2(this->d_hrimg_ngpu[cur_device]->getData(),
         this->d_fttotim_ngpu[cur_device]->getData(), this->nfft * this->nfft,
         fact, this->current_context->get_device(cur_device));

  }
}

//////////////////////////////
// PYRAMID WAVEFRONT SENSOR //
//////////////////////////////

// It starts by looking for the type of sensor. By default it assumes
// a pyramid wfs. The pyramid can also be explicitely asked for, or
// a roof prism can be asked for as well.
int sutra_wfs_pyr_pyrhr::comp_generic() {
  /*
   //___________________________________________________________________
   //  PYRAMID SENSOR MODEL

   This code generates pupil images as seen from behind a pyramid wavefront sensor
   algorithm:
   for (i=0;i<mod_pts;i++) {
   get phase and multiply by exp(i*modu)
   do fft
   apply field stop
   myltiply by exp(i*pyramid) where pyramid is the pyramid shape
   do fft-1
   do abs2
   add to previous modulation image
   }
   do fft
   multiply by sinc (pixes transfer function)
   take 1 pixels over nrebin pixels in the image
   normalize
   add noise
   */
  current_context->set_activeDevice(device,1);

  carmaSafeCall(
      cudaMemset(this->d_hrimg->getData(), 0,
                 sizeof(float) * this->d_hrimg->getNbElem()));
  //this->npup = 1;

  for (std::vector<carma_obj<float> *>::iterator it =
      this->d_screen_ngpu.begin(); this->d_screen_ngpu.end() != it; it++) {
    carma_obj<float> *tmp_screen = this->d_gs->d_phase->d_screen;
    if (*it != tmp_screen) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      (*it)->copyFrom(tmp_screen->getData(), tmp_screen->getNbElem());
    }
  }

  for (std::vector<carma_obj<float> *>::iterator it =
      this->d_hrimg_ngpu.begin(); this->d_hrimg_ngpu.end() != it; it++) {
    if (*it != d_hrimg) {
      current_context->set_activeDevice((*it)->getDevice(),1);
      carmaSafeCall(
          cudaMemset((*it)->getData(), 0, sizeof(float) * (*it)->getNbElem()));
    }
  }

  for (int cpt = 0; cpt < this->npup; cpt++) {
    //int cpt = 0;
    comp_modulation(cpt);
  }

  current_context->set_activeDevice(device,1);

  long dims_data2[3];
  dims_data2[0] = 2;
  dims_data2[1] = nfft;
  dims_data2[2] = nfft;
  carma_obj<float> *tmp_vector=new carma_obj<float>(current_context, dims_data2);

  for (std::vector<carma_obj<float> *>::iterator it =
      this->d_hrimg_ngpu.begin(); this->d_hrimg_ngpu.end() != it; it++) {
    if (*it != d_hrimg) {
      //tmp_vector->copyFrom((*it)->getData(), (*it)->getNbElem());
      d_hrimg->axpy(1.0f, tmp_vector, 1, 1);
    }
  }
  delete tmp_vector;

  cfillrealp(this->d_fttotim->getData(), this->d_hrimg->getData(),
             this->d_hrimg->getNbElem(),
             this->current_context->get_device(device));

  carma_fft(this->d_fttotim->getData(), this->d_fttotim->getData(), -1,
            *this->d_camplipup->getPlan());

  pyr_submask(this->d_fttotim->getData(), this->d_sincar->getData(), this->nfft,
              this->current_context->get_device(device));

  carma_fft(this->d_fttotim->getData(), this->d_fttotim->getData(), 1,
            *this->d_camplipup->getPlan());

  cgetrealp(this->d_hrimg->getData(), this->d_fttotim->getData(),
            this->d_hrimg->getNbElem(),
            this->current_context->get_device(device));

  pyr_fillbinimg(this->d_binimg->getData(), this->d_hrimg->getData(),
                 this->nfft / this->nrebin, this->nfft, this->nrebin, false,
                 this->current_context->get_device(device));

  pyr_subsum(this->d_subsum->getData(), this->d_binimg->getData(),
             this->d_validsubsx->getData(), this->d_validsubsy->getData(),
             this->nfft / this->nrebin, this->nvalid,
             this->current_context->get_device(device));

  int blocks, threads;
//  getNumBlocksAndThreads(current_context->get_device(device), this->nvalid,
//      blocks, threads);
  this->current_context->set_activeDevice(device,1);
  sumGetNumBlocksAndThreads(this->nvalid,
                            this->current_context->get_device(device), blocks,
                            threads);

  reduce(this->nvalid, threads, blocks, this->d_subsum->getData(),
         this->d_subsum->getData());

  pyr_fact(this->d_binimg->getData(), this->nphot, this->d_subsum->getData(),
           this->nfft / this->nrebin, 1,
           this->current_context->get_device(device));

  // add noise
  if (this->noise > -1) {
    //cout << "adding poisson noise" << endl;
    this->d_bincube->prng('P');
  }
  if (this->noise > 0) {
    //cout << "adding detector noise" << endl;
    this->d_bincube->prng('N', this->noise, 1.0f);
  }

  //Done in getpyr
//  pyr_subsum(this->d_subsum->getData(), this->d_binimg->getData(),
//             this->d_validsubsx->getData(), this->d_validsubsy->getData(),
//             this->nfft / this->nrebin, this->nvalid,
//             this->current_context->get_device(device));

  return EXIT_SUCCESS;

}

int sutra_wfs_pyr_pyrhr::comp_image() {

  current_context->set_activeDevice(device,1);
  int result = comp_generic();
  return result;
}

int sutra_wfs_pyr_pyrhr::slopes_geom(int type, float *slopes) {

  current_context->set_activeDevice(device,1);
  /*
   normalization notes :
   ���� = 0.17 (��/D)^2 (D/r_0)^(5/3) , ���� en radians d'angle
   �� = sqrt(0.17 (��/D)^2 (D/r_0)^(5/3)) * 206265 , �� en secondes

   // computing subaperture phase difference at edges

   todo : integrale( x * phase ) / integrale (x^2);
   with x = span(-0.5,0.5,npixels)(,-:1:npixels) * subap_diam * 2 * pi / lambda / 0.206265
   */
  if (type == 0) {
    // this is to convert in arcsec
    //> 206265* 0.000001/ 2 / 3.14159265 = 0.0328281
    // it would have been the case if the phase was given in radiants
    // but it is given in microns so normalization factor is
    // just 206265* 0.000001 = 0.206265

    //float alpha = 0.0328281 * this->d_gs->lambda / this->subapd;
    float alpha = 0.206265 / this->subapd;
    phase_reduce(this->nphase, this->nvalid,
                 this->d_gs->d_phase->d_screen->getData(), slopes,
                 this->d_phasemap->getData(), alpha);
  }

  if (type == 1) {

    //float alpha = 0.0328281 * this->d_gs->lambda / this->subapd;
    float alpha = 0.206265 / this->subapd;
    phase_derive(this->nphase * this->nphase * this->nvalid,
                 this->nphase * this->nphase, this->nvalid, this->nphase,
                 this->d_gs->d_phase->d_screen->getData(), slopes,
                 this->d_phasemap->getData(), this->d_pupil->getData(), alpha,
                 this->d_fluxPerSub->getData());

  }

  return EXIT_SUCCESS;
}

int sutra_wfs_pyr_pyrhr::slopes_geom(int type) {
  this->slopes_geom(type, this->d_slopes->getData());

  return EXIT_SUCCESS;
}
