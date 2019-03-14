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

class ProgressBar {
  int prev = 0, count = 0, max;
  int ndigits = 0;
  double progress = 0;
  int barWidth = 42;
  std::string desc;
  std::chrono::system_clock::time_point start;

 public:
  ProgressBar(int i, const std::string &desc = "");
  void update();
  void finish();
};

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

////////////////////////////////////////////////////////////////////////////
//! CUT bool type
////////////////////////////////////////////////////////////////////////////
enum CUTBoolean { CUTFalse = 0, CUTTrue = 1 };

// We define these calls here, so the user doesn't need to include __FILE__ and
// __LINE__ The advantage is the developers gets to use the inline function so
// they can debug
#ifdef DEBUG
#define carmaSafeCallNoSync(err) __carmaSafeCallNoSync(err, __FILE__, __LINE__)
#define carmaSafeCall(err) __carmaSafeCall((err), #err, __FILE__, __LINE__)
#define carmaSafeDeviceSynchronize() \
  __carmaSafeDeviceSynchronize(__FILE__, __LINE__)
#define carmafftSafeCall(err) __carmafftSafeCall(err, __FILE__, __LINE__)
#define carmaCheckError(err) __carmaCheckError(err, __FILE__, __LINE__)
#define carmaCheckMsg(msg) __carmaCheckMsg(msg, __FILE__, __LINE__)
#define carmaSafeMalloc(mallocCall) \
  __carmaSafeMalloc((mallocCall), __FILE__, __LINE__)
#else
#define carmaSafeCallNoSync(err) err
#define carmaSafeCall(err) err
#define carmaSafeDeviceSynchronize() cudaDeviceSynchronize()
#define carmafftSafeCall(err) err
#define cutilCheckError(err) err
#define carmaCheckMsg(msg)
#define cutilSafeMalloc(mallocCall) (mallocCall)
#endif

#ifndef MIN
#define MIN(a, b) ((a < b) ? a : b)
#endif
#ifndef MAX
#define MAX(a, b) ((a > b) ? a : b)
#endif

inline unsigned int nextPow2(unsigned int x) {
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return ++x;
}

inline bool isPow2(unsigned int x) { return ((x & (x - 1)) == 0); }

class carma_device;
void getNumBlocksAndThreads(carma_device *device, int n, int &blocks,
                            int &threads);
void sumGetNumBlocksAndThreads(int n, carma_device *device, int &blocks,
                               int &threads);
template <class T_data>
int find_nnz(T_data *d_data, int *tmp_colind, int N, int *d_nnz, int &h_nnz,
             carma_device *device);
template <class T_data>
int fill_sparse_vect(T_data *dense_data, int *colind_sorted, T_data *values,
                     int *colind, int *rowind, int nnz, carma_device *device);
int floattodouble(float *idata, double *odata, int N, carma_device *device);
int doubletofloat(double *idata, float *odata, int N, carma_device *device);
int printMemInfo();
template <typename T_data>
int fill_array_with_value(T_data *d_data, T_data value, int N,
                          carma_device *device);

#ifdef CAN_DO_HALF
int copyFromFloatToHalf(const float *data, half *dest, int N,
                        carma_device *device);
int copyFromHalfToFloat(const half *d_data, float *h_dest, int N,
                        carma_device *device);
half *float2halfArray(float *source, int N, carma_device *device);
float *half2floatArray(half *source, int N, carma_device *device);
#endif

void carma_start_profile();
void carma_stop_profile();

// NOTE: "(%s:%i) : " allows Eclipse to directly jump to the file at the right
// line when the user double clicks on the error line in the Output pane. Like
// any compile error.

inline void __carmaSafeCallNoSync(cudaError err, const char *file,
                                  const int line) {
  if (cudaSuccess != err) {
    fprintf(stderr, "(%s:%i) : carmaSafeCallNoSync() Runtime API error : %s.\n",
            file, line, cudaGetErrorString(err));
    // exit(EXIT_FAILURE);
    throw cudaGetErrorString(err);
  }
}

inline void __carmaSafeCall(cudaError err, const char *code, const char *file,
                            const int line) {
  if (cudaSuccess != err) {
    fprintf(stderr, "[%s:%i] %s\n carmaSafeCall() Runtime API error : %s.\n",
            file, line, code, cudaGetErrorString(err));
    // exit(EXIT_FAILURE);
    throw cudaGetErrorString(err);
  }
}

inline void __carmaSafeDeviceSynchronize(const char *file, const int line) {
  cudaError err = cudaDeviceSynchronize();
  if (cudaSuccess != err) {
    fprintf(stderr,
            "(%s:%i) : cudaDeviceSynchronize() Driver API error : %s.\n", file,
            line, cudaGetErrorString(err));
    // exit(EXIT_FAILURE);
    throw cudaGetErrorString(err);
  }
}

inline void __carmafftSafeCall(cufftResult err, const char *file,
                               const int line) {
  if (CUFFT_SUCCESS != err) {
    fprintf(stderr, "(%s:%i) : carmafftSafeCall() CUFFT error.\n", file, line);
    // exit(EXIT_FAILURE);
    throw "carmafftSafeCall() CUFFT error";
  }
}

inline void __carmaCheckError(CUTBoolean err, const char *file,
                              const int line) {
  if (CUTTrue != err) {
    fprintf(stderr, "(%s:%i) : CUTIL CUDA error.\n", file, line);
    throw "carmafftSafeCall() CUTIL CUDA error";
  }
}

inline void __carmaCheckMsg(const char *errorMessage, const char *file,
                            const int line) {
  cudaError_t err = cudaGetLastError();
  if (cudaSuccess != err) {
    fprintf(stderr, "(%s:%i) : carmaCheckMsg() CUTIL CUDA error : %s : %s.\n",
            file, line, errorMessage, cudaGetErrorString(err));
    throw cudaGetErrorString(err);
  }
#ifdef DEBUG
  err = cudaDeviceSynchronize();
  if (cudaSuccess != err) {
    fprintf(stderr,
            "(%s:%i) : carmaCheckMsg cudaDeviceSynchronize error: %s : %s.\n",
            file, line, errorMessage, cudaGetErrorString(err));
    throw cudaGetErrorString(err);
  }
#endif
}
inline void __carmaSafeMalloc(void *pointer, const char *file, const int line) {
  if (!(pointer)) {
    fprintf(stderr, "(%s:%i) : cutilSafeMalloc host malloc failure\n", file,
            line);
    throw "cutilSafeMalloc() cutilSafeMalloc host malloc failure";
  }
}

#endif  // _CARMA_UTILS_H_
