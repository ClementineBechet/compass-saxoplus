#include <thrust/device_ptr.h>
#include <thrust/sort.h>
#include "carma_utils.cuh"
#include "carma_utils.h"

template <class T_data>
__global__ void find_nnz_krnl(T_data *d_data, int *colind, int *d_nnz, int N) {
  int *sdata = SharedMemory<int>();
  int tid = threadIdx.x + blockDim.x * blockIdx.x;
  int sid = threadIdx.x;
  if (tid == 0) d_nnz[0] = 0;

  // Load shared memory with 1 if d_data[tid]!= 0, with 0 else
  if (tid < N) {
    sdata[sid] = (d_data[tid] != 0);
    colind[tid] = (sdata[sid]) ? tid : N + tid;  // Init colind for further sort
  } else {
    sdata[sid] = 0;
  }
  __syncthreads();
  reduce_krnl(sdata, blockDim.x, sid);
  __syncthreads();

  if (threadIdx.x == 0)
    //		intensities[blockIdx.x] = sdata[0];
    atomicAdd(d_nnz, sdata[0]);
}
template <class T_data>
int find_nnz(T_data *d_data, int *colind, int N, int *d_nnz, int &h_nnz,
             carma_device *device) {
  int nthreads = 0, nblocks = 0;
  getNumBlocksAndThreads(device, N, nblocks, nthreads);
  dim3 grid(nblocks), threads(nthreads);
  int smemSize = nthreads * sizeof(int);

  find_nnz_krnl<<<grid, threads, smemSize>>>(d_data, colind, d_nnz, N);
  carmaCheckMsg("find_nnz_krnl<<<>>> execution failed\n");

  // wrap raw pointer with a device_ptr
  thrust::device_ptr<int> dev_ptr(colind);

  thrust::sort(dev_ptr, dev_ptr + N);
  carmaSafeCall(cudaMemcpy(&h_nnz, d_nnz, sizeof(int), cudaMemcpyDeviceToHost));

  return EXIT_SUCCESS;
}

template int find_nnz<float>(float *d_data, int *colind, int N, int *d_nnz,
                             int &h_nnz, carma_device *device);
template int find_nnz<double>(double *d_data, int *colind, int N, int *d_nnz,
                              int &h_nnz, carma_device *device);

template <class T_data>
__global__ void fill_sparse_vect_krnl(T_data *dense_data, int *colind_sorted,
                                      T_data *values, int *colind, int *rowind,
                                      int nnz) {
  int tid = threadIdx.x + blockDim.x * blockIdx.x;
  if (tid == 0) rowind[0] = 0;
  if (tid == 1) rowind[1] = nnz;

  // Load shared memory with 1 if d_data[tid]!= 0, with 0 else
  if (tid < nnz) {
    values[tid] = dense_data[colind_sorted[tid]];
    colind[tid] = colind_sorted[tid];
  }
  __syncthreads();
}
template <class T_data>
int fill_sparse_vect(T_data *dense_data, int *colind_sorted, T_data *values,
                     int *colind, int *rowind, int nnz, carma_device *device) {
  int nthreads = 0, nblocks = 0;
  getNumBlocksAndThreads(device, nnz, nblocks, nthreads);
  dim3 grid(nblocks), threads(nthreads);

  fill_sparse_vect_krnl<<<grid, threads>>>(dense_data, colind_sorted, values,
                                           colind, rowind, nnz);
  carmaCheckMsg("fill_sparse_vect_krnl<<<>>> execution failed\n");
  return EXIT_SUCCESS;
}
template int fill_sparse_vect<float>(float *dense_data, int *colind_sorted,
                                     float *values, int *colind, int *rowind,
                                     int nnz, carma_device *device);
template int fill_sparse_vect<double>(double *dense_data, int *colind_sorted,
                                      double *values, int *colind, int *rowind,
                                      int nnz, carma_device *device);

__global__ void floattodouble_krnl(float *i_data, double *o_data, int N) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  while (tid < N) {
    o_data[tid] = (double)i_data[tid];
    tid += blockDim.x * gridDim.x;
  }
}

__global__ void doubletofloat_krnl(double *i_data, float *o_data, int N) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  while (tid < N) {
    o_data[tid] = (float)i_data[tid];
    tid += blockDim.x * gridDim.x;
  }
}

int floattodouble(float *i_data, double *o_data, int N, carma_device *device) {
  int nthreads = 0, nblocks = 0;
  getNumBlocksAndThreads(device, N, nblocks, nthreads);
  dim3 grid(nblocks), threads(nthreads);
  floattodouble_krnl<<<grid, threads>>>(i_data, o_data, N);
  carmaCheckMsg("floattodouble_krnl<<<>>> execution failed\n");

  return EXIT_SUCCESS;
}

int doubletofloat(double *i_data, float *o_data, int N, carma_device *device) {
  int nthreads = 0, nblocks = 0;
  getNumBlocksAndThreads(device, N, nblocks, nthreads);
  dim3 grid(nblocks), threads(nthreads);
  doubletofloat_krnl<<<grid, threads>>>(i_data, o_data, N);
  carmaCheckMsg("floattodouble_krnl<<<>>> execution failed\n");

  return EXIT_SUCCESS;
}

__global__ void float2halfArray_krnl(float *source, half *dest, int N) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  while (tid < N) {
    dest[tid] = __float2half(source[tid]);
    tid += blockDim.x * gridDim.x;
  }
}

half *float2halfArray(float *source, int N, carma_device *device) {
  int nthreads = 0, nblocks = 0;
  half *dest;
  carmaSafeCall(cudaMalloc((void **)&(dest), sizeof(half) * N));
  getNumBlocksAndThreads(device, N, nblocks, nthreads);
  dim3 grid(nblocks), threads(nthreads);
  float2halfArray_krnl<<<grid, threads>>>(source, dest, N);
  carmaCheckMsg("float2halfArray_krnl\n");

  return dest;
}

__global__ void half2floatArray_krnl(half *source, float *dest, int N) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  while (tid < N) {
    dest[tid] = __half2float(source[tid]);
    tid += blockDim.x * gridDim.x;
  }
}

float *half2floatArray(half *source, int N, carma_device *device) {
  int nthreads = 0, nblocks = 0;
  float *dest;
  carmaSafeCall(cudaMalloc((void **)&(dest), sizeof(float) * N));
  getNumBlocksAndThreads(device, N, nblocks, nthreads);
  dim3 grid(nblocks), threads(nthreads);
  half2floatArray_krnl<<<grid, threads>>>(source, dest, N);
  carmaCheckMsg("half2floatArray_krnl\n");

  return dest;
}

__global__ void copyFromFloatToHalf_krnl(const float *data, half *dest, int N) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  while (tid < N) {
    dest[tid] = __float2half(data[tid]);
    tid += blockDim.x * gridDim.x;
  }
}

int copyFromFloatToHalf(const float *h_data, half *d_dest, int N,
                        carma_device *device) {
  int nthreads = 0, nblocks = 0;
  float *d_data;
  carmaSafeCall(cudaMalloc((void **)&d_data, sizeof(float) * N));
  carmaSafeCall(
      cudaMemcpy(d_data, h_data, sizeof(float) * N, cudaMemcpyHostToDevice));

  getNumBlocksAndThreads(device, N, nblocks, nthreads);
  dim3 grid(nblocks), threads(nthreads);
  copyFromFloatToHalf_krnl<<<grid, threads>>>(d_data, d_dest, N);
  carmaCheckMsg("copyFromFloatToHalf_krnl\n");
  cudaFree(d_data);

  return EXIT_SUCCESS;
}

__global__ void copyFromHalfToFloat_krnl(const half *data, float *dest, int N) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  while (tid < N) {
    dest[tid] = __half2float(data[tid]);
    tid += blockDim.x * gridDim.x;
  }
}

int copyFromHalfToFloat(const half *d_data, float *h_dest, int N,
                        carma_device *device) {
  int nthreads = 0, nblocks = 0;
  float *d_dest;
  carmaSafeCall(cudaMalloc((void **)&d_dest, sizeof(float) * N));
  getNumBlocksAndThreads(device, N, nblocks, nthreads);
  dim3 grid(nblocks), threads(nthreads);
  copyFromHalfToFloat_krnl<<<grid, threads>>>(d_data, d_dest, N);
  carmaCheckMsg("copyFromHalfToFloat_krnl\n");
  carmaSafeCall(
      cudaMemcpy(h_dest, d_dest, sizeof(float) * N, cudaMemcpyDeviceToHost));

  cudaFree(d_dest);

  return EXIT_SUCCESS;
}
