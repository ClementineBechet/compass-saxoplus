/**
 * \class carma_context
 *
 * \ingroup libcarma
 *
 * \brief this class provides the context in which carma_obj are created
 *
 * \author $Author: dg, as $
 *
 * \version $Revision: 1.0 $
 *
 * \date $Date: 2011/01/28$
 *
 */

#ifndef _CARMA_CONTEXT_H_
#define _CARMA_CONTEXT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <driver_types.h>
#include <vector_types.h>
#include <cuda_runtime_api.h>

#include <carma_utils.h>
#include <carma_cublas.h>

using namespace std;

class carma_device {
protected:
  int id;
  cudaDeviceProp properties;
  float compute_perf;
  float sm_per_multiproc;
  bool p2p_activate;

public:
  carma_device(int devid);
  carma_device(const carma_device& device);
  ~carma_device();

  int
  get_id() {
    return id;
  }
  cudaDeviceProp
  get_properties() {
    return properties;
  }
  float
  get_compute_perf() {
    return compute_perf;
  }
  float
  get_sm_per_multiproc() {
    return sm_per_multiproc;
  }
  bool
  isGPUCapableP2P() {
    return (bool) (properties.major >= 2);
  }

  bool
  isP2P_active() {
    return p2p_activate;
  }

};

class carma_context {
protected:
  int ndevice;
  vector<carma_device *> devices;
  int activeDevice;
  int** can_access_peer;
  cublasHandle_t cublasHandle;

public:
  carma_context();
  carma_context(const carma_context& cntxt);
  ~carma_context();

  int
  get_ndevice() {
    return ndevice;
  }
  carma_device *
  get_device(int dev) {
    return devices[dev];
  }
  int
  get_activeDevice() {
    return activeDevice;
  }
  string
  get_activeDeviceStr();
  int
  set_activeDevice(int newDevice, int silent = 1);
  int
  set_activeDeviceForCpy(int newDevice, int silent = 1);
  int
  get_maxGflopsDeviceId();
  cublasHandle_t
  get_cublasHandle() {
    return cublasHandle;
  }
};

/// from /usr/local/cuda/samples/common/inc/helper_cuda.h
inline int
ConvertSMVer2Cores(int major, int minor) {
  // Defines for GPU Architecture types (using the SM version to determine the # of cores per SM
  typedef struct {
    int SM; // 0xMm (hexidecimal notation), M = SM Major version, and m = SM minor version
    int Cores;
  } sSMtoCores;

  sSMtoCores nGpuArchCoresPerSM[] = { { 0x10, 8 }, // Tesla Generation (SM 1.0) G80 class
      { 0x11, 8 }, // Tesla Generation (SM 1.1) G8x class
      { 0x12, 8 }, // Tesla Generation (SM 1.2) G9x class
      { 0x13, 8 }, // Tesla Generation (SM 1.3) GT200 class
      { 0x20, 32 }, // Fermi Generation (SM 2.0) GF100 class
      { 0x21, 48 }, // Fermi Generation (SM 2.1) GF10x class
      { 0x30, 192 }, // Kepler Generation (SM 3.0) GK10x class
      { 0x35, 192 }, // Kepler Generation (SM 3.5) GK11x class
      { -1, -1 } };

  int index = 0;

  while (nGpuArchCoresPerSM[index].SM != -1) {
    if (nGpuArchCoresPerSM[index].SM == ((major << 4) + minor)) {
      return nGpuArchCoresPerSM[index].Cores;
    }

    index++;
  }

  // If we don't find the values, we default use the previous one to run properly
  printf(
      "MapSMtoCores for SM %d.%d is undefined.  Default to use %d Cores/SM\n",
      major, minor, nGpuArchCoresPerSM[7].Cores);
  return nGpuArchCoresPerSM[7].Cores;
}
#endif // _CARMA_CONTEXT_H_
