#include <sutra_wfs_sh.h>
#include <sutra_ao_utils.h>
#include <carma_utils.h>


sutra_wfs_sh::sutra_wfs_sh(carma_context *context, sutra_sensors *sensors, long nxsub,
    long nvalid, long npix, long nphase, long nrebin, long nfft, long ntot,
    long npup, float pdiam, float nphotons, int lgs, int device) {
  this->type = "sh";
  this->d_camplipup = sensors->d_camplipup;
  this->d_camplifoc = sensors->d_camplifoc;
  this->d_fttotim = sensors->d_fttotim;
  this->d_ftkernel = 0L;
  this->d_pupil = 0L;
  this->d_bincube = 0L;
  this->d_binimg = 0L;
  this->d_subsum = 0L;
  this->d_offsets = 0L;
  this->d_fluxPerSub = 0L;
  this->d_sincar = 0L;
  this->d_hrmap = 0L;
  this->d_isvalid = 0L;
  this->d_slopes = 0L;
  this->image_telemetry = 0L;
  this->d_phasemap = 0L;
  this->d_binmap = 0L;
  this->d_validsubsx = 0L;
  this->d_validsubsy = 0L;
  this->d_istart = 0L;
  this->d_jstart = 0L;

  this->d_gs = 0L;

  this->current_context = context;

  this->noise = 0;
  this->nxsub = nxsub;
  this->nvalid = nvalid;
  this->npix = npix;
  this->nphase = nphase;
  this->nrebin = nrebin;
  this->nfft = nfft;
  this->ntot = ntot;
  this->npup = npup;
  this->subapd = pdiam;
  this->nphot = nphotons;
  this->lgs = (lgs == 1 ? true : false);
  this->device = device;
  context->set_activeDevice(device,1);
  this->nmaxhr = nvalid;
  this->nffthr = 1;

  this->kernconv = false;

  long *dims_data1 = new long[2];
  dims_data1[0] = 1;
  long *dims_data2 = new long[3];
  dims_data2[0] = 2;
  long *dims_data3 = new long[4];
  dims_data3[0] = 3;

  dims_data2[1] = npix * nxsub;
  dims_data2[2] = npix * nxsub;

  this->d_binimg = new carma_obj<float>(context, dims_data2);
  // using 1 stream for telemetry
  this->image_telemetry = new carma_host_obj<float>(dims_data2, MA_PAGELOCK, 1);

  dims_data3[1] = nfft;
  dims_data3[2] = nfft;

  int nslaves = 0;

  if (nslaves > 0)
    dims_data3[3] = nvalid / (nslaves + 1);
  else
    dims_data3[3] = nvalid;

  int mdims[2];

  //this->d_submask = new carma_obj<float>(context, dims_data3); // Useless for SH

  //this->d_camplipup = new carma_obj<cuFloatComplex>(context, dims_data3);
  //this->d_camplifoc = new carma_obj<cuFloatComplex>(context, dims_data3);
  mdims[0] = (int) dims_data3[1];
  mdims[1] = (int) dims_data3[2];

  //int vector_dims[3] = {mdims[0],mdims[1],(int)dims_data3[3]};
  vector<int> vdims(dims_data3 + 1, dims_data3 + 4);

  if (sensors->campli_plans.find(vdims) == sensors->campli_plans.end()) {
    //DEBUG_TRACE("Creating FFT plan : %d %d %d",mdims[0],mdims[1],dims_data3[3]);printMemInfo();
    cufftHandle *plan = (cufftHandle*) malloc(sizeof(cufftHandle)); // = this->d_camplipup->getPlan(); ///< FFT plan
    cufftSafeCall(
        cufftPlanMany(plan, 2 ,mdims,NULL,1,0,NULL,1,0, CUFFT_C2C ,(int)dims_data3[3]));

    sensors->campli_plans.insert(
        pair<vector<int>, cufftHandle*>(vdims, plan));

    this->campli_plan = plan;
    //DEBUG_TRACE("FFT plan created");printMemInfo();
  } else {
    //DEBUG_TRACE("FFT plan already exists : %d %d %d",mdims[0],mdims[1],dims_data3[3]);
    this->campli_plan = sensors->campli_plans.at(vdims);
  }

  dims_data3[1] = npix;
  dims_data3[2] = npix;

  this->d_bincube = new carma_obj<float>(context, dims_data3);

  this->nstreams = 1;
  while (nvalid % this->nstreams != 0)
    nstreams--;
  cerr << "wfs uses " << nstreams << " streams" << endl;
  this->streams = new carma_streams(nstreams);

  dims_data1[1] = 2 * nvalid;
  this->d_slopes = new carma_obj<float>(context, dims_data1);

  if (this->ntot != this->nfft) {
    // this is the big array => we use nmaxhr and treat it sequentially
    int mnmax = 500;
    if (nvalid > 2 * mnmax) {
      nmaxhr = compute_nmaxhr(nvalid);

      this->nffthr = (
          nvalid % this->nmaxhr == 0 ?
              nvalid / this->nmaxhr : nvalid / this->nmaxhr + 1);
    }
    dims_data3[1] = ntot;
    dims_data3[2] = ntot;
    dims_data3[3] = nmaxhr;

    //this->d_fttotim = new carma_obj<cuFloatComplex>(context, dims_data3);

    mdims[0] = (int) dims_data3[1];
    mdims[1] = (int) dims_data3[2];
    int vector_dims[3] = { mdims[0], mdims[1], (int) dims_data3[3] };
    vector<int> vdims(vector_dims,
                      vector_dims + sizeof(vector_dims) / sizeof(int));

    if (sensors->fttotim_plans.find(vdims) == sensors->fttotim_plans.end()) {
      cufftHandle *plan = (cufftHandle*) malloc(sizeof(cufftHandle)); // = this->d_fttotim->getPlan(); ///< FFT plan
      //DEBUG_TRACE("Creating FFT plan :%d %d %d",mdims[0],mdims[1],dims_data3[3]);printMemInfo();
      cufftSafeCall(
          cufftPlanMany(plan, 2 ,mdims,NULL,1,0,NULL,1,0,CUFFT_C2C , (int)dims_data3[3]));
      sensors->fttotim_plans.insert(
          pair<vector<int>, cufftHandle*>(vdims, plan));
      this->fttotim_plan = plan;
      //DEBUG_TRACE("FFT plan created : ");printMemInfo();
    } else {
      //DEBUG_TRACE("FFT plan already exists %d %d %d",mdims[0],mdims[1],dims_data3[3]);
      this->fttotim_plan = sensors->fttotim_plans.at(vdims);
    }
    dims_data1[1] = nfft * nfft;
    this->d_hrmap = new carma_obj<int>(context, dims_data1);

  } else {
    if (this->lgs) {

      dims_data3[1] = ntot;
      dims_data3[2] = ntot;
      dims_data3[3] = nvalid;
      // this->d_fttotim = new carma_obj<cuFloatComplex>(context, dims_data3);
      mdims[0] = (int) dims_data3[1];
      mdims[1] = (int) dims_data3[2];
      int vector_dims[3] = { mdims[0], mdims[1], (int) dims_data3[3] };
      vector<int> vdims(vector_dims,
                        vector_dims + sizeof(vector_dims) / sizeof(int));

      if (sensors->fttotim_plans.find(vdims) == sensors->fttotim_plans.end()) {
        //DEBUG_TRACE("Creating FFT plan : %d %d %d",mdims[0],mdims[1],dims_data3[3]);printMemInfo();
        cufftHandle *plan = (cufftHandle*) malloc(sizeof(cufftHandle)); // = this->d_fttotim->getPlan(); ///< FFT plan
        cufftSafeCall(
            cufftPlanMany(plan, 2 ,mdims,NULL,1,0,NULL,1,0,CUFFT_C2C , (int)dims_data3[3]));
        sensors->fttotim_plans.insert(
            pair<vector<int>, cufftHandle*>(vdims, plan));
        this->fttotim_plan = plan;
        //DEBUG_TRACE("FFT plan created : ");printMemInfo();
      } else {
        //DEBUG_TRACE("FFT plan already exists : %d %d %d",mdims[0],mdims[1],dims_data3[3]);
        this->fttotim_plan = sensors->fttotim_plans.at(vdims);
      }
    }
  }

  dims_data2[1] = ntot;
  dims_data2[2] = ntot;
  this->d_ftkernel = new carma_obj<cuFloatComplex>(context, dims_data2);

  dims_data2[1] = npup;
  dims_data2[2] = npup;
  this->d_pupil = new carma_obj<float>(context, dims_data2);

  dims_data2[1] = nphase;
  dims_data2[2] = nphase;
  this->d_offsets = new carma_obj<float>(context, dims_data2);

  dims_data1[1] = nxsub;
  this->d_istart = new carma_obj<int>(context, dims_data1);
  this->d_jstart = new carma_obj<int>(context, dims_data1);

  dims_data2[1] = nrebin * nrebin;
  dims_data2[2] = npix * npix;
  this->d_binmap = new carma_obj<int>(context, dims_data2);

  dims_data1[1] = nvalid;
  this->d_subsum = new carma_obj<float>(context, dims_data1);

  this->d_fluxPerSub = new carma_obj<float>(context, dims_data1);
  this->d_validsubsx = new carma_obj<int>(context, dims_data1);
  this->d_validsubsy = new carma_obj<int>(context, dims_data1);

  dims_data2[1] = nxsub;
  dims_data2[2] = nxsub;
  this->d_isvalid = new carma_obj<int>(context, dims_data2);

  dims_data2[1] = nphase * nphase;
  dims_data2[2] = nvalid;
  this->d_phasemap = new carma_obj<int>(context, dims_data2);
}


sutra_wfs_sh::~sutra_wfs_sh() {
  current_context->set_activeDevice(device,1);

  if (this->d_ftkernel != 0L)
    delete this->d_ftkernel;

  if (this->d_pupil != 0L)
    delete this->d_pupil;
  if (this->d_bincube != 0L)
    delete this->d_bincube;
  if (this->d_binimg != 0L)
    delete this->d_binimg;
  if (this->d_subsum != 0L)
    delete this->d_subsum;
  if (this->d_offsets != 0L)
    delete this->d_offsets;
  if (this->d_fluxPerSub != 0L)
    delete this->d_fluxPerSub;
  if (this->d_sincar != 0L)
    delete this->d_sincar;
  if (this->d_hrmap != 0L)
    delete this->d_hrmap;

  if (this->d_isvalid != 0L)
    delete this->d_isvalid;
  if (this->d_slopes != 0L)
    delete this->d_slopes;

  if (this->image_telemetry != 0L)
    delete this->image_telemetry;

  if (this->d_phasemap != 0L)
    delete this->d_phasemap;
  if (this->d_binmap != 0L)
    delete this->d_binmap;
  if (this->d_validsubsx != 0L)
    delete this->d_validsubsx;
  if (this->d_validsubsy != 0L)
    delete this->d_validsubsy;
  if (this->d_istart != 0L)
    delete this->d_istart;
  if (this->d_jstart != 0L)
    delete this->d_jstart;

  if (this->lgs)
    delete this->d_gs->d_lgs;

  delete this->d_gs;

  delete this->streams;

  //delete this->current_context;
}

int sutra_wfs_sh::wfs_initarrays(int *phasemap, int *hrmap, int *binmap,
    float *offsets, float *pupil, float *fluxPerSub, int *isvalid,
    int *validsubsx, int *validsubsy, int *istart, int *jstart,
    cuFloatComplex *kernel) {
  current_context->set_activeDevice(device,1);
  this->d_phasemap->host2device(phasemap);
  this->d_offsets->host2device(offsets);
  this->d_pupil->host2device(pupil);
  this->d_binmap->host2device(binmap);
  this->d_fluxPerSub->host2device(fluxPerSub);
  if (this->ntot != this->nfft)
    this->d_hrmap->host2device(hrmap);
  this->d_validsubsx->host2device(validsubsx);
  this->d_validsubsy->host2device(validsubsy);
  this->d_isvalid->host2device(isvalid);
  this->d_istart->host2device(istart);
  this->d_jstart->host2device(jstart);
  this->d_ftkernel->host2device(kernel);

  return EXIT_SUCCESS;
}


/////////////////////////////////////////////////////////
// COMPUTATION OF THE SHACK-HARTMANN WAVEFRONT SENSOR  //
/////////////////////////////////////////////////////////

int sutra_wfs_sh::comp_generic() {
  current_context->set_activeDevice(device,1);

  // segment phase and fill cube of complex ampli with exp(i*phase_seg)
  fillcamplipup(this->d_camplipup->getData(),
      this->d_gs->d_phase->d_screen->getData(), this->d_offsets->getData(),
      this->d_pupil->getData(), this->d_gs->scale, this->d_istart->getData(),
      this->d_jstart->getData(), this->d_validsubsx->getData(),
      this->d_validsubsy->getData(), this->nphase,
      this->d_gs->d_phase->d_screen->getDims(1), this->nfft,
      this->nphase * this->nphase * this->nvalid,
      this->current_context->get_device(device));
  // do fft of the cube  
  carma_fft(this->d_camplipup->getData(), this->d_camplifoc->getData(), 1,
      *this->campli_plan);//*this->d_camplipup->getPlan());

  // get the hrimage by taking the | |^2
  // keep it in amplifoc to save mem space
  abs2c(this->d_camplifoc->getData(), this->d_camplifoc->getData(),
      this->d_camplifoc->getDims(1) * this->d_camplifoc->getDims(2)
          * this->d_camplifoc->getDims(3), current_context->get_device(device));

  //set bincube to 0 or noise
  cutilSafeCall(
      cudaMemset(this->d_bincube->getData(), 0,
          sizeof(float) * this->d_bincube->getNbElem()));
  // increase fov if required
  // and fill bincube with data from hrimg
  // we need to do this sequentially if nvalid > nmaxhr to
  // keep raesonable mem occupancy
  if (this->ntot != this->nfft) {

    for (int cc = 0; cc < this->nffthr; cc++) {
      cutilSafeCall(
          cudaMemset(this->d_fttotim->getData(), 0,
              sizeof(cuFloatComplex) * this->d_fttotim->getNbElem()));

      int indxstart1, indxstart2 = 0, indxstart3;

      if ((cc == this->nffthr - 1) && (this->nvalid % this->nmaxhr != 0)) {
        indxstart1 = this->d_camplifoc->getNbElem()
            - this->nfft * this->nfft * this->nmaxhr;
        if (this->lgs)
          indxstart2 = this->ntot * this->nvalid - this->ntot * this->nmaxhr;
        indxstart3 = this->d_bincube->getNbElem()
            - this->npix * this->npix * this->nmaxhr;
      } else {
        indxstart1 = this->nfft * this->nfft * this->nmaxhr * cc;
        if (this->lgs)
          indxstart2 = this->ntot * this->nmaxhr * cc;
        indxstart3 = this->npix * this->npix * this->nmaxhr * cc;
      }

      cuFloatComplex *data = this->d_camplifoc->getData();
      indexfill(this->d_fttotim->getData(), &(data[indxstart1]),
          this->d_hrmap->getData(), this->nfft, this->ntot,
          this->nfft * this->nfft * this->nmaxhr,
          this->current_context->get_device(device));

      if (this->lgs) {
        // compute lgs spot on the fly from binned profile image
        this->d_gs->d_lgs->lgs_makespot(
            this->current_context->get_device(device), indxstart2);
        // convolve with psf
        carma_fft(this->d_fttotim->getData(), this->d_fttotim->getData(), 1,
            *this->fttotim_plan);//*this->d_fttotim->getPlan());

        convolve(this->d_fttotim->getData(),
            this->d_gs->d_lgs->d_ftlgskern->getData(),
            this->d_fttotim->getNbElem(),
            this->current_context->get_device(device));

        carma_fft(this->d_fttotim->getData(), this->d_fttotim->getData(), -1,
        		*this->fttotim_plan);//*this->d_fttotim->getPlan());

      }

      if (this->kernconv) {
        carma_fft(this->d_fttotim->getData(), this->d_fttotim->getData(), 1,
        		*this->fttotim_plan);//*this->d_fttotim->getPlan());

        convolve_cube(this->d_fttotim->getData(), this->d_ftkernel->getData(),
            this->d_fttotim->getNbElem(), this->d_ftkernel->getNbElem(),
            this->current_context->get_device(device));

        carma_fft(this->d_fttotim->getData(), this->d_fttotim->getData(), -1,
        		*this->fttotim_plan);//*this->d_fttotim->getPlan());

      }

      float *data2 = this->d_bincube->getData();
      //fprintf(stderr, "[%s@%d]: I'm here!\n", __FILE__, __LINE__);
      if (this->nstreams > 1) {
        fillbincube_async(this->streams, &(data2[indxstart3]),
            this->d_fttotim->getData(), this->d_binmap->getData(),
            this->ntot * this->ntot, this->npix * this->npix,
            this->nrebin * this->nrebin, this->nmaxhr,
            this->current_context->get_device(device));
      } else {
        fillbincube(&(data2[indxstart3]), this->d_fttotim->getData(),
            this->d_binmap->getData(), this->ntot * this->ntot,
            this->npix * this->npix, this->nrebin * this->nrebin, this->nmaxhr,
            this->current_context->get_device(device));
        //fprintf(stderr, "[%s@%d]: I'm here!\n", __FILE__, __LINE__);
      }
    }
  } else {
    if (this->lgs) {
      this->d_gs->d_lgs->lgs_makespot(this->current_context->get_device(device),
          0);

      carma_fft(this->d_camplifoc->getData(), this->d_fttotim->getData(), 1,
    		  *this->fttotim_plan);//*this->d_fttotim->getPlan());

      convolve(this->d_fttotim->getData(),
          this->d_gs->d_lgs->d_ftlgskern->getData(),
          this->d_fttotim->getNbElem(),
          this->current_context->get_device(device));

      carma_fft(this->d_fttotim->getData(), this->d_fttotim->getData(), -1,
    		  *this->fttotim_plan);//*this->d_fttotim->getPlan());

      if (this->nstreams > 1) {
        fillbincube_async(this->streams, this->d_bincube->getData(),
            this->d_fttotim->getData(), this->d_binmap->getData(),
            this->nfft * this->nfft, this->npix * this->npix,
            this->nrebin * this->nrebin, this->nvalid,
            this->current_context->get_device(device));
      } else {
        fillbincube(this->d_bincube->getData(), this->d_fttotim->getData(),
            this->d_binmap->getData(), this->nfft * this->nfft,
            this->npix * this->npix, this->nrebin * this->nrebin, this->nvalid,
            this->current_context->get_device(device));
      }
    } else {
      if (this->kernconv) {
        carma_fft(this->d_camplifoc->getData(), this->d_camplifoc->getData(), 1,
        		*this->campli_plan);//*this->d_camplipup->getPlan());

        convolve_cube(this->d_camplifoc->getData(), this->d_ftkernel->getData(),
            this->d_camplifoc->getNbElem(), this->d_ftkernel->getNbElem(),
            this->current_context->get_device(device));

        carma_fft(this->d_camplifoc->getData(), this->d_camplifoc->getData(),
            -1, *this->campli_plan);//*this->d_camplipup->getPlan());
      }

      if (this->nstreams > 1) {
        fillbincube_async(this->streams, this->d_bincube->getData(),
            this->d_camplifoc->getData(), this->d_binmap->getData(),
            this->nfft * this->nfft, this->npix * this->npix,
            this->nrebin * this->nrebin, this->nvalid,
            this->current_context->get_device(device));
      } else {
        fillbincube(this->d_bincube->getData(), this->d_camplifoc->getData(),
            this->d_binmap->getData(), this->nfft * this->nfft,
            this->npix * this->npix, this->nrebin * this->nrebin, this->nvalid,
            this->current_context->get_device(device));
      }
    }

  }
  // normalize images :
  // get the sum value per subap

  if (this->nstreams > 1) {
    subap_reduce_async(this->npix * this->npix, this->nvalid, this->streams,
        this->d_bincube->getData(), this->d_subsum->getData());
  } else {
    subap_reduce(this->d_bincube->getNbElem(), this->npix * this->npix,
        this->nvalid, this->d_bincube->getData(), this->d_subsum->getData(),
        this->current_context->get_device(device));
  }

  if (this->nstreams > 1) {
    subap_norm_async(this->d_bincube->getData(), this->d_bincube->getData(),
        this->d_fluxPerSub->getData(), this->d_subsum->getData(), this->nphot,
        this->npix * this->npix, this->d_bincube->getNbElem(), this->streams,
        current_context->get_device(device));
  } else {
    // multiply each subap by nphot*fluxPersub/sumPerSub
    subap_norm(this->d_bincube->getData(), this->d_bincube->getData(),
        this->d_fluxPerSub->getData(), this->d_subsum->getData(), this->nphot,
        this->npix * this->npix, this->d_bincube->getNbElem(),
        current_context->get_device(device));
  }
  //fprintf(stderr, "[%s@%d]: I'm here!\n", __FILE__, __LINE__);

  // add noise
  if (this->noise > -1) {
    //cout << "adding poisson noise" << endl;
    this->d_bincube->prng('P');
  }
  if (this->noise > 0) {
    //cout << "adding detector noise" << endl;
    this->d_bincube->prng('N', this->noise, 1.0f);
  }
  return EXIT_SUCCESS;
}

//////////////////////////////
// PYRAMID WAVEFRONT SENSOR //
//////////////////////////////

// It starts by looking for the type of sensor. By default it assumes
// a pyramid wfs. The pyramid can also be explicitely asked for, or
// a roof prism can be asked for as well.

int sutra_wfs_sh::fill_binimage() {
  current_context->set_activeDevice(device,1);
  fillbinimg(this->d_binimg->getData(), this->d_bincube->getData(),
      this->npix, this->nvalid, this->npix * this->nxsub,
      this->d_validsubsx->getData(), this->d_validsubsy->getData(),
      0, this->current_context->get_device(device));
  return EXIT_SUCCESS;
}

int sutra_wfs_sh::comp_image() {
  current_context->set_activeDevice(device,1);

  int result = comp_generic();

  if (noise > 0)
    this->d_binimg->prng('N', this->noise);

  if (result == EXIT_SUCCESS)
    fillbinimg(this->d_binimg->getData(), this->d_bincube->getData(),
               this->npix, this->nvalid, this->npix * this->nxsub,
               this->d_validsubsx->getData(), this->d_validsubsy->getData(),
               0/*(this->noise > 0)*/,
               this->current_context->get_device(device));

  return result;
}

int sutra_wfs_sh::comp_image_tele() {
  current_context->set_activeDevice(device,1);

  int result = comp_generic();

  if (noise > 0)
    this->d_binimg->prng('N', this->noise);

  if (result == EXIT_SUCCESS)
    fillbinimg_async(this->image_telemetry, this->d_binimg->getData(),
        this->d_bincube->getData(), this->npix, this->nvalid,
        this->npix * this->nxsub, this->d_validsubsx->getData(),
        this->d_validsubsy->getData(), this->d_binimg->getNbElem(), false,
        this->current_context->get_device(device));

  return result;
}

int sutra_wfs_sh::slopes_geom(int type, float *slopes) {
  current_context->set_activeDevice(device,1);
  /*
   normalization notes :
   σ² = 0.17 (λ/D)^2 (D/r_0)^(5/3) , σ² en radians d'angle
   σ = sqrt(0.17 (λ/D)^2 (D/r_0)^(5/3)) * 206265 , σ en secondes

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

int sutra_wfs_sh::slopes_geom(int type) {
  this->slopes_geom(type, this->d_slopes->getData());

  return EXIT_SUCCESS;
}