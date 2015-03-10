#include <sutra_wfs_pyr.h>
#include <sutra_ao_utils.h>
#include <carma_utils.h>

sutra_wfs_pyr::sutra_wfs_pyr(carma_context *context, sutra_sensors *sensors, long nxsub,
    long nvalid, long npix, long nphase, long nrebin, long nfft, long ntot,
    long npup, float pdiam, float nphotons, int lgs, int device) {
  this->d_camplipup = sensors->d_camplipup;
  this->d_camplifoc = sensors->d_camplifoc;
  this->d_fttotim = sensors->d_fttotim;
  this->d_ftkernel = 0L;
  this->d_pupil = 0L;
  this->d_hrimg = 0L;
  this->d_bincube = 0L;
  this->d_binimg = 0L;
  this->d_subsum = 0L;
  this->d_offsets = 0L;
  this->d_fluxPerSub = 0L;
  this->d_sincar = 0L;
  this->d_submask = 0L;
  this->d_hrmap = 0L;
  this->d_isvalid = 0L;
  this->d_slopes = 0L;
  this->image_telemetry = 0L;
  this->d_phasemap = 0L;
  this->d_validsubsx = 0L;
  this->d_validsubsy = 0L;
  this->d_psum = 0L;
  this->d_phalfxy = 0L;
  this->d_poffsets = 0L;
  this->pyr_cx = 0L;
  this->pyr_cy = 0L;

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

  dims_data2[1] = npix * nxsub + 3;
  dims_data2[2] = npix * nxsub + 3;

  this->d_binimg = new carma_obj<float>(context, dims_data2);
  // using 1 stream for telemetry
  this->image_telemetry = new carma_host_obj<float>(dims_data2, MA_PAGELOCK, 1);

  dims_data3[1] = nfft;
  dims_data3[2] = nfft;
  dims_data3[3] = 4;

  this->d_hrimg = new carma_obj<float>(context, dims_data3); // Useless for SH

  int mdims[2];

  dims_data2[1] = ntot;
  dims_data2[2] = ntot;
  this->d_submask = new carma_obj<float>(context, dims_data2);
  this->d_camplipup = new carma_obj<cuFloatComplex>(context, dims_data2);
  this->d_camplifoc = new carma_obj<cuFloatComplex>(context, dims_data2);
  cufftHandle *plan = this->d_camplipup->getPlan(); ///< FFT plan
  cufftSafeCall(cufftPlan2d(plan, dims_data2[1], dims_data2[2], CUFFT_C2C));

  dims_data3[1] = nfft / nrebin;
  dims_data3[2] = nfft / nrebin;

  this->d_bincube = new carma_obj<float>(context, dims_data3);

  this->nstreams = 1;
  while (nvalid % this->nstreams != 0)
    nstreams--;
  cerr << "wfs uses " << nstreams << " streams" << endl;
  this->streams = new carma_streams(nstreams);

  dims_data1[1] = 2 * nvalid;
  this->d_slopes = new carma_obj<float>(context, dims_data1);

  dims_data3[1] = nfft;
  dims_data3[2] = nfft;
  dims_data3[3] = 4;
  this->d_fttotim = new carma_obj<cuFloatComplex>(context, dims_data3);
  mdims[0] = (int) dims_data3[1];
  mdims[1] = (int) dims_data3[2];
  plan = this->d_fttotim->getPlan(); ///< FFT plan
  cufftSafeCall(
      cufftPlanMany(plan, 2 ,mdims,NULL,1,0,NULL,1,0,CUFFT_C2C , (int)dims_data3[3]));

  dims_data2[1] = ntot;
  dims_data2[2] = ntot;
  this->d_pupil = new carma_obj<float>(context, dims_data2);

  dims_data1[1] = npup;
  this->pyr_cx = new carma_host_obj<int>(dims_data1, MA_WRICOMB);
  this->pyr_cy = new carma_host_obj<int>(dims_data1, MA_WRICOMB);
  dims_data2[1] = nfft;
  dims_data2[2] = nfft;
  this->d_poffsets = new carma_obj<cuFloatComplex>(context, dims_data2);
  dims_data2[1] = ntot;
  dims_data2[2] = ntot;
  this->d_phalfxy = new carma_obj<cuFloatComplex>(context, dims_data2);
  dims_data2[1] = nfft;
  dims_data2[2] = nfft;
  this->d_sincar = new carma_obj<float>(context, dims_data2);

  dims_data1[1] = nvalid;
  this->d_subsum = new carma_obj<float>(context, dims_data1);

  this->d_psum = new carma_obj<float>(context, dims_data1);

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

sutra_wfs_pyr::~sutra_wfs_pyr() {
  current_context->set_activeDevice(device,1);
  if (this->d_camplipup != 0L)
    delete this->d_camplipup;
  if (this->d_camplifoc != 0L)
    delete this->d_camplifoc;

  if (this->d_fttotim != 0L)
    delete this->d_fttotim;

  if (this->d_ftkernel != 0L)
    delete this->d_ftkernel;

  if (this->d_pupil != 0L)
    delete this->d_pupil;
  if (this->d_hrimg != 0L)
    delete this->d_hrimg;
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
  if (this->d_submask != 0L)
    delete this->d_submask;
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
  if (this->d_validsubsx != 0L)
    delete this->d_validsubsx;
  if (this->d_validsubsy != 0L)
    delete this->d_validsubsy;

  if (this->d_psum != 0L)
    delete this->d_psum;
  if (this->d_phalfxy != 0L)
    delete this->d_phalfxy;
  if (this->d_poffsets != 0L)
    delete this->d_poffsets;
  if (this->pyr_cx != 0L)
    delete this->pyr_cx;
  if (this->pyr_cy != 0L)
    delete this->pyr_cy;

  if (this->lgs)
    delete this->d_gs->d_lgs;

  delete this->d_gs;

  delete this->streams;

  //delete this->current_context;
}


int sutra_wfs_pyr::wfs_initarrays(cuFloatComplex *halfxy, cuFloatComplex *offsets,
    float *focmask, float *pupil, int *isvalid, int *cx, int *cy, float *sincar,
    int *phasemap, int *validsubsx, int *validsubsy) {
  current_context->set_activeDevice(device,1);
  this->d_phalfxy->host2device(halfxy);
  this->d_poffsets->host2device(offsets);
  this->d_submask->host2device(focmask);
  this->d_pupil->host2device(pupil);
  this->pyr_cx->fill_from(cx);
  this->pyr_cy->fill_from(cy);
  this->d_isvalid->host2device(isvalid);
  this->d_sincar->host2device(sincar);
  this->d_phasemap->host2device(phasemap);
  this->d_validsubsx->host2device(validsubsx);
  this->d_validsubsy->host2device(validsubsy);

  return EXIT_SUCCESS;
}