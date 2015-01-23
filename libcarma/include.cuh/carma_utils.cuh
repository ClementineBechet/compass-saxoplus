#ifndef _CARMA_UTILS_CUH_
#define _CARMA_UTILS_CUH_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cufft.h>

#include <driver_types.h>
#include <vector_types.h>

#include <cuda_runtime_api.h>
#include <cuda.h>
#include <cufft.h>

template<class T>
struct SharedMemory {
  __device__
  inline operator T*() {
    extern __shared__ int __smem[];
    return (T*) __smem;
  }

  __device__
  inline operator const T*() const {
    extern __shared__ int __smem[];
    return (T*) __smem;
  }
};

// specialize for double to avoid unaligned memory
// access compile errors
template<>
struct SharedMemory<double> {
  __device__
  inline operator double*() {
    extern __shared__ double __smem_d[];
    return (double*) __smem_d;
  }

  __device__
  inline operator const double*() const {
    extern __shared__ double __smem_d[];
    return (double*) __smem_d;
  }
};
template<>
struct SharedMemory<float> {
  __device__
  inline operator float*() {
    extern __shared__ float __smem_f[];
    return (float*) __smem_f;
  }

  __device__
  inline operator const float*() const {
    extern __shared__ float __smem_f[];
    return (float*) __smem_f;
  }
};

template<class T>
__device__ inline void mswap(T & a, T & b) {
  T tmp = a;
  a = b;
  b = tmp;
}

template<class T>
__inline__ __device__ void reduce_krnl( T *sdata, int size, int n) {
  if (size & (size - 1)) { // test if size is not a power of 2
    unsigned int s;
    if ((size & 1) != 0)
      s = size / 2 + 1; //(size&1)==size%2
    else
      s = size / 2;
    unsigned int s_old = size;
    while (s > 0) {
      if ((n < s) && (n + s < s_old)) {
        sdata[n] += sdata[n + s];
      }
      //__threadfence_block();
      //__threadfence();
      __syncthreads();
     s_old = s;
      s /= 2;
      if ((2 * s < s_old) && (s != 0))
        s += 1;
    }
  } else {
    // do reduction in shared mem
    for (unsigned int s = size / 2; s > 0; s >>= 1) {
      if (n < s) {
        sdata[n] += sdata[n + s];
      }
      //__threadfence_block();
      //__threadfence();
      __syncthreads();
    }
  }
}

#endif //_CARMA_UTILS_CUH_