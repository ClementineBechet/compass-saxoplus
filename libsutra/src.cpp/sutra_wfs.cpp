#include <sutra_wfs.h>
#include <sutra_wfs_geom.h>
#include <sutra_wfs_pyr_roof.h>
#include <sutra_wfs_pyr_pyr4.h>
#include <sutra_wfs_sh.h>
#include <sutra_ao_utils.h>
#include <carma_utils.h>

int compute_nmaxhr(long nvalid) {
  // this is the big array => we use nmaxhr and treat it sequentially

  int mnmax = 100;
  int nmaxhr = mnmax;
  if (nvalid > 2 * mnmax) {
    int tmp0 = nvalid % mnmax;
    int tmp = 0;
    for (int cc = 1; cc < mnmax / 5; cc++) {
      tmp = nvalid % (mnmax + cc);
      if ((tmp > tmp0) || (tmp == 0)) {
        if (tmp == 0)
          tmp0 = 2 * mnmax;
        else
          tmp = tmp0;

        nmaxhr = mnmax + cc;
      }
    }
    return nmaxhr;
  }
  return nvalid;
}

int sutra_wfs::wfs_initgs(sutra_sensors *sensors, float xpos, float ypos,
                          float lambda, float mag, long size, float noise,
                          long seed) {
  current_context->set_activeDevice(device,1);
  this->d_gs = new sutra_source(current_context, xpos, ypos, lambda, mag, size,
                                "wfs", this->device);
  this->noise = noise;
  if (noise > -1) {
    this->d_bincube->init_prng(seed);
    this->d_bincube->prng('N', noise, 0.0f);
  }
  if (noise > 0) {
    this->d_binimg->init_prng(seed);
    this->d_binimg->prng('N', noise, 0.0f);
  }

  if (this->lgs) {
    this->d_gs->d_lgs = new sutra_lgs(current_context, sensors, this->nvalid,
                                      this->ntot, this->nmaxhr);
    this->d_gs->lgs = this->lgs;
  }

  return EXIT_SUCCESS;
}

int sutra_wfs::load_kernels(float *lgskern) {
  current_context->set_activeDevice(device,1);
  if (this->lgs)
    this->d_gs->d_lgs->load_kernels(lgskern,
                                    this->current_context->get_device(device));

  return EXIT_SUCCESS;
}

int sutra_wfs::sensor_trace(sutra_atmos *yatmos) {
  current_context->set_activeDevice(device,1);
  //do raytracing to get the phase
  this->d_gs->raytrace(yatmos);

  // same with textures
  // apparently not working for large sizes...
  //this->d_gs->raytrace_shm(yatmos);

  return EXIT_SUCCESS;
}

int sutra_wfs::sensor_trace(sutra_dms *ydm, int rst) {
  current_context->set_activeDevice(device,1);
  //do raytracing to get the phase
  this->d_gs->raytrace(ydm, rst);

  return EXIT_SUCCESS;
}

int sutra_wfs::sensor_trace(sutra_atmos *yatmos, sutra_dms *ydms) {
  this->d_gs->raytrace(yatmos);
  this->d_gs->raytrace(ydms, 0);

  return EXIT_SUCCESS;
}

sutra_sensors::sutra_sensors(carma_context *context, char **type, int nwfs,
                             long *nxsub, long *nvalid, long *npix,
                             long *nphase, long *nrebin, long *nfft, long *ntot,
                             long *npup, float *pdiam, float *nphot, int *lgs,
                             int device) {
  this->current_context = context;
  this->device = device;
  current_context->set_activeDevice(device,1);
  //DEBUG_TRACE("Before create sensors : ");printMemInfo();
  int maxnfft = nfft[0];
  int maxntot = ntot[0];
  int maxnvalid = nvalid[0];
  int wfs4nfft = 0;
  int wfs4ntot = 0;
  int is_lgs = (lgs[0] > 0 ? 1 : 0);
  for (int i = 1; i < nwfs; i++) {
    if (ntot[i] > maxntot) {
      maxntot = ntot[i];
      wfs4ntot = i;
    }
    if (nfft[i] > maxnfft) {
      maxnfft = nfft[i];
      wfs4nfft = i;
    }
	if (ntot[i] == nfft[i]) {
		if (nvalid[i] > maxnvalid) {
				maxnvalid = nvalid[i];
		}
	}
	if(lgs[i]>0)
		is_lgs = 1;
	}
	//DEBUG_TRACE("maxntot : %d maxnfft : %d maxnvalid : %d nvalid[wfs4nfft] : %d nmaxhr : %d\n ",maxntot,maxnfft,maxnvalid,nvalid[wfs4nfft],compute_nmaxhr(nvalid[wfs4ntot]));
	long dims_data3[4] = {3,maxnfft,maxnfft,maxnvalid};
	this->d_camplifoc = new carma_obj<cuFloatComplex>(context, dims_data3);
	this->d_camplipup = new carma_obj<cuFloatComplex>(context, dims_data3);


  dims_data3[1] = maxntot;
  dims_data3[2] = maxntot;
  dims_data3[3] = compute_nmaxhr(nvalid[wfs4ntot]);
  if (maxnvalid > dims_data3[3])
    dims_data3[3] = maxnvalid;
  this->d_fttotim = new carma_obj<cuFloatComplex>(context, dims_data3);
  if (is_lgs) {
    this->d_ftlgskern = new carma_obj<cuFloatComplex>(context, dims_data3);
    this->d_lgskern = new carma_obj<float>(context, dims_data3);
  } else {
    this->d_ftlgskern = 0L;
    this->d_lgskern = 0L;
  }
//	DEBUG_TRACE("After creating sensors arrays : ");printMemInfo();
  for (int i = 0; i < nwfs; i++) {
    sutra_wfs *wfs = NULL;
    if (strcmp(type[i],"sh") == 0)
      wfs = new sutra_wfs_sh(context, this, nxsub[i], nvalid[i], npix[i],
                             nphase[i], nrebin[i], nfft[i], ntot[i], npup[i],
                             pdiam[i], nphot[i], lgs[i], device);
    if (strcmp(type[i],"pyr") == 0)
      wfs = new sutra_wfs_pyr_pyr4(context, this, nxsub[i], nvalid[i], npix[i],
                                   nphase[i], nrebin[i], nfft[i], ntot[i], npup[i],
                                   pdiam[i], nphot[i], lgs[i], device);
    if (strcmp(type[i],"roof") == 0)
      wfs = new sutra_wfs_pyr_roof(context, this, nxsub[i], nvalid[i], npix[i],
                                   nphase[i], nrebin[i], nfft[i], ntot[i], npup[i],
                                   pdiam[i], nphot[i], lgs[i], device);
    d_wfs.push_back(wfs);

//		DEBUG_TRACE("After creating wfs #%d : ",i);printMemInfo();
  }
//	DEBUG_TRACE("Final sensors : ");printMemInfo();
}

sutra_sensors::sutra_sensors(carma_context *context, int nwfs, long *nxsub,
                             long *nvalid, long *nphase, long npup,
                             float *pdiam, int device) {
  this->current_context = context;
  this->device = device;
  current_context->set_activeDevice(device,1);
  DEBUG_TRACE("device %d", device);

  for (int i = 0; i < nwfs; i++) {
    d_wfs.push_back(
        new sutra_wfs_geom(context, nxsub[i], nvalid[i], nphase[i], npup,
                           pdiam[i], device));
  }
  this->d_camplifoc = 0L;
  this->d_camplipup = 0L;
  this->d_fttotim = 0L;
  this->d_lgskern = 0L;
  this->d_ftlgskern = 0L;

}

sutra_sensors::~sutra_sensors() {
  current_context->set_activeDevice(device,1);
//  for (size_t idx = 0; idx < (this->d_wfs).size(); idx++) {
  while ((this->d_wfs).size() > 0) {
    delete this->d_wfs.back();
    d_wfs.pop_back();
  }
  map<vector<int>, cufftHandle*>::iterator it;
  for (it = campli_plans.begin(); it != campli_plans.end(); it++) {
    cufftDestroy(*it->second);
    free(it->second);
  }
  for (it = fttotim_plans.begin(); it != fttotim_plans.end(); it++) {
    cufftDestroy(*it->second);
    free(it->second);
  }
  for (it = ftlgskern_plans.begin(); it != ftlgskern_plans.end(); it++) {
    cufftDestroy(*it->second);
    free(it->second);
  }

  if (this->d_camplifoc != 0L)
    delete this->d_camplifoc;
  if (this->d_camplipup != 0L)
    delete this->d_camplipup;
  if (this->d_fttotim != 0L)
    delete this->d_fttotim;
  if (this->d_lgskern != 0L)
    delete this->d_lgskern;
  if (this->d_ftlgskern != 0L)
    delete this->d_ftlgskern;
}

int sutra_sensors::sensors_initgs(float *xpos, float *ypos, float *lambda,
                                  float *mag, long *size, float *noise,
                                  long *seed) {
  current_context->set_activeDevice(device,1);
  for (size_t idx = 0; idx < (this->d_wfs).size(); idx++) {
    (this->d_wfs)[idx]->wfs_initgs(this, xpos[idx], ypos[idx], lambda[idx],
                                   mag[idx], size[idx], noise[idx], seed[idx]);
  }
  return EXIT_SUCCESS;
}
int sutra_sensors::sensors_initgs(float *xpos, float *ypos, float *lambda,
                                  float *mag, long *size, float *noise) {
  current_context->set_activeDevice(device,1);
  for (size_t idx = 0; idx < (this->d_wfs).size(); idx++) {
    (this->d_wfs)[idx]->wfs_initgs(this, xpos[idx], ypos[idx], lambda[idx],
                                   mag[idx], size[idx], noise[idx], 1234 * idx);
  }
  return EXIT_SUCCESS;
}
int sutra_sensors::sensors_initgs(float *xpos, float *ypos, float *lambda,
                                  float *mag, long *size) {
  current_context->set_activeDevice(device,1);
  for (size_t idx = 0; idx < (this->d_wfs).size(); idx++) {
    (this->d_wfs)[idx]->wfs_initgs(this, xpos[idx], ypos[idx], lambda[idx],
                                   mag[idx], size[idx], -1, 1234);
  }
  return EXIT_SUCCESS;
}

