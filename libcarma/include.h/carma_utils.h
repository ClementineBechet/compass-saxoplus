// -----------------------------------------------------------------------------
//  This file is part of COMPASS <https://anr-compass.github.io/compass/>
//
//  Copyright (C) 2011-2023 COMPASS Team <https://github.com/ANR-COMPASS>
//  All rights reserved.

// -----------------------------------------------------------------------------

//! \file      carma_utils.h
//! \ingroup   libcarma
//! \brief     this file provides tools to CarmaObj
//! \author    COMPASS Team <https://github.com/ANR-COMPASS>
//! \version   5.4.4
//! \date      2022/01/24


#ifndef _CARMA_UTILS_H_
#define _CARMA_UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <driver_types.h>
#include <vector_types.h>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <cuda.h>
#include <cuda_fp16.h>
#include <cuda_runtime_api.h>
#include <cufft.h>

#include <carma_indicators.hpp>

#ifdef USE_OCTOPUS
#include <Cacao.h>
#endif

#define CARMA_PI 3.1415926535897932384626433832

struct doubleint {
  int start;
  int nbInflu;
};

template <class T>
struct tuple_t {
  int pos;
  T data;
};

namespace carma_utils {
template <typename T>
inline std::string to_string(const T &n) {
  std::ostringstream stm;
  stm << n;
  return stm.str();
}
template <typename T>
inline T from_string(const std::string &myString) {
  std::istringstream buffer(myString);
  T value;
  buffer >> value;
  return value;
}
void inline split(std::vector<std::string> &tokens, const std::string &text,
                  char sep) {
  std::string::size_type start = 0, end = 0;
  while ((end = text.find(sep, start)) != std::string::npos) {
    tokens.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(text.substr(start));
}

}  // namespace carma_utils

#ifdef DEBUG
#define DEBUG_TRACE(fmt, args...) \
  fprintf(stderr, "[%s@%d]: " fmt "\n", __FILE__, __LINE__, ##args)
#else
#define DEBUG_TRACE(fmt, args...) \
  fprintf(stderr, "[%s@%d]: " fmt "\n", __FILE__, __LINE__, ##args)
#endif

#define CAST(type, new_var, var) type new_var = dynamic_cast<type>(var)
#define SCAST(type, new_var, var) type new_var = static_cast<type>(var)

// We define these calls here, so the user doesn't need to include __FILE__ and
// __LINE__ The advantage is the developers gets to use the inline function so
// they can debug
#ifdef DEBUG
#define carma_safe_call_no_sync(err) __carma_safe_call_no_sync(err, __FILE__, __LINE__)
#define carma_safe_call(err) __carma_safe_call((err), #err, __FILE__, __LINE__)
#define carma_safe_device_synchronize() \
  __carma_safe_device_synchronize(__FILE__, __LINE__)
#define carmafft_safe_call(err) __carmafft_safe_call(err, __FILE__, __LINE__)
#define carma_check_msg(msg) __carma_check_msg(msg, __FILE__, __LINE__)
#define carma_safe_malloc(mallocCall) \
  __carma_safe_malloc((mallocCall), __FILE__, __LINE__)
#else
#define carma_safe_call_no_sync(err) err
#define carma_safe_call(err) err
#define carma_safe_device_synchronize() cudaDeviceSynchronize()
#define carmafft_safe_call(err) err
#define cutil_check_error(err) err
#define carma_check_msg(msg)
#define cutil_safe_malloc(mallocCall) (mallocCall)
#endif

#ifndef MIN
#define MIN(a, b) ((a < b) ? a : b)
#endif
#ifndef MAX
#define MAX(a, b) ((a > b) ? a : b)
#endif

inline unsigned int next_pow2(unsigned int x) {
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return ++x;
}

inline bool is_pow2(unsigned int x) { return ((x & (x - 1)) == 0); }

class CarmaDevice;
void get_num_blocks_and_threads(CarmaDevice *device, int n, int &blocks,
                            int &threads);
void sum_get_num_blocks_and_threads(int n, CarmaDevice *device, int &blocks,
                               int &threads);
template <class T_data>
int find_nnz(T_data *d_data, int *tmp_colind, int N, int *d_nnz, int &h_nnz,
             CarmaDevice *device);
template <class T_data>
int fill_sparse_vect(T_data *dense_data, int *colind_sorted, T_data *values,
                     int *colind, int *rowind, int nnz, CarmaDevice *device);
int float_to_double(float *idata, double *odata, int N, CarmaDevice *device);
int double_to_float(double *idata, float *odata, int N, CarmaDevice *device);
int print_mem_info();
template <typename T_data>
int fill_array_with_value(T_data *d_data, T_data value, int N,
                          CarmaDevice *device);

#ifdef CAN_DO_HALF
int copy_from_float_to_half(const float *data, half *dest, int N,
                        CarmaDevice *device);
int copy_from_half_to_float(const half *d_data, float *h_dest, int N,
                        CarmaDevice *device);
half *float_to_half_array(float *source, int N, CarmaDevice *device);
float *half_to_float_array(half *source, int N, CarmaDevice *device);
#endif

void carma_start_profile();
void carma_stop_profile();

// NOTE: "(%s:%i) : " allows Eclipse to directly jump to the file at the right
// line when the user double clicks on the error line in the Output pane. Like
// any compile error.

inline void __carma_safe_call_no_sync(cudaError err, const char *file,
                                  const int line) {
  if (cudaSuccess != err) {
    fprintf(stderr, "(%s:%i) : carma_safe_call_no_sync() Runtime API error : %s.\n",
            file, line, cudaGetErrorString(err));
    // exit(EXIT_FAILURE);
    throw cudaGetErrorString(err);
  }
}

inline void __carma_safe_call(cudaError err, const char *code, const char *file,
                            const int line) {
  if (cudaSuccess != err) {
    fprintf(stderr, "[%s:%i] %s\n carma_safe_call() Runtime API error : %s.\n",
            file, line, code, cudaGetErrorString(err));
    // exit(EXIT_FAILURE);
    throw cudaGetErrorString(err);
  }
}

inline void __carma_safe_device_synchronize(const char *file, const int line) {
  cudaError err = cudaDeviceSynchronize();
  if (cudaSuccess != err) {
    fprintf(stderr,
            "(%s:%i) : cudaDeviceSynchronize() Driver API error : %s.\n", file,
            line, cudaGetErrorString(err));
    // exit(EXIT_FAILURE);
    throw cudaGetErrorString(err);
  }
}

inline void __carmafft_safe_call(cufftResult err, const char *file,
                               const int line) {
  if (CUFFT_SUCCESS != err) {
    fprintf(stderr, "(%s:%i) : carmafft_safe_call() CUFFT error.\n", file, line);
    // exit(EXIT_FAILURE);
    throw "carmafft_safe_call() CUFFT error";
  }
}

inline void __carma_check_msg(const char *error_message, const char *file,
                            const int line) {
  cudaError_t err = cudaGetLastError();
  if (cudaSuccess != err) {
    fprintf(stderr, "(%s:%i) : carma_check_msg() CUTIL CUDA error : %s : %s.\n",
            file, line, error_message, cudaGetErrorString(err));
    throw cudaGetErrorString(err);
  }
#ifdef DEBUG
  err = cudaDeviceSynchronize();
  if (cudaSuccess != err) {
    fprintf(stderr,
            "(%s:%i) : carma_check_msg cudaDeviceSynchronize error: %s : %s.\n",
            file, line, error_message, cudaGetErrorString(err));
    throw cudaGetErrorString(err);
  }
#endif
}
inline void __carma_safe_malloc(void *pointer, const char *file, const int line) {
  if (!(pointer)) {
    fprintf(stderr, "(%s:%i) : cutil_safe_malloc host malloc failure\n", file,
            line);
    throw "cutil_safe_malloc() cutil_safe_malloc host malloc failure";
  }
}

#endif  // _CARMA_UTILS_H_
