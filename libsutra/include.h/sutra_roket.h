#ifndef _SUTRA_ROKET_H_
#define _SUTRA_ROKET_H_

#include <carma.h>
#include <carma_obj.h>
#include <sutra_ao_utils.h>
#include <sutra_rtc.h>
#include <sutra_wfs.h>
#include <sutra_target.h>
#include <sutra_turbu.h>
#include <sutra_telescope.h>
#include <sutra_ao_utils.h>

class sutra_roket {
  public:
    carma_context *current_context;
    int device;
    float gain; // Loop gain
    int nfilt; // Number of filtered modes
    int nactus; // number of actuators
    int nmodes; // number of modes
    int iterk; // current iteration number
    int niter;
    int loopcontroller; // index of the loop controller
    int geocontroller; // index of the geo controller
    // sutra objects to supervise
    sutra_rtc *rtc;
    sutra_sensors *sensors;
    sutra_target *target;
    sutra_telescope *tel;
    sutra_atmos *atm;
    sutra_dms *dms;
    sutra_controller_ls *loopcontrol;
    sutra_controller_geo *geocontrol;

    // Projection matrices
    carma_obj<float> *d_P;
    carma_obj<float> *d_Btt;

    // Error contributors buffers
    carma_obj<float> *d_noise;
    carma_obj<float> *d_nonlinear;
    carma_obj<float> *d_tomo;
    carma_obj<float> *d_filtered;
    carma_obj<float> *d_alias;
    carma_obj<float> *d_bandwidth;
    float fitting;

    // Residual error buffers
    carma_obj<float> *d_fullErr;
    carma_obj<float> *d_err1;
    carma_obj<float> *d_err2;
    // Command loop backup
    carma_obj<float> *d_bkup_com;
    // Target screen backup
    carma_obj<float> * d_bkup_screen;
    // Additional buffers
    carma_obj<float> *d_commanded;
    carma_obj<float> *d_modes;
    carma_obj<float> *d_filtmodes;
    carma_obj<float> *d_tmpdiff;
    // Loop filter matrix
    carma_obj<float> *d_gRD;
    // R*D matrix
    carma_obj<float> *d_RD;
    // PSF ortho
    carma_obj<float> *d_psfortho;
    //carma_obj<float> *d_psfse;


  public:
    sutra_roket(carma_context *context, int device, sutra_rtc *rtc, sutra_sensors *sensors,
                    sutra_target *target, sutra_dms *dms, sutra_telescope *tel, sutra_atmos *atm, int loopcontroller, int geocontroller,
                    int nactus, int nmodes, int nfilt, int niter, float *Btt, float *P, float *gRD, float *RD);
    ~sutra_roket();

    int
    compute_breakdown();
    int
    save_loop_state();
    int
    restore_loop_state();
    int
    apply_loop_filter(carma_obj<float> *d_odata, carma_obj<float> *d_idata1,
                      carma_obj<float> *d_idata2, float gain, int k);
};

int
separate_modes(float *modes, float *filtmodes, int nmodes, int nfilt, carma_device *device);

#endif