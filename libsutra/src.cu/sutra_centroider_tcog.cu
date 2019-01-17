#include <sutra_centroider_tcog.h>
#include <carma_utils.cuh>

template <class T, int Nthreads>
__global__ void centroids(T *d_img, T *d_centroids, T *ref, int *validx,
                          int *validy, T *d_intensities, T threshold,
                          unsigned int npix, unsigned int size, T scale,
                          T offset, unsigned int nelem_thread) {
  if (blockDim.x > Nthreads) {
    if (threadIdx.x == 0) printf("Wrong size argument\n");
    return;
  }
  // Specialize BlockReduce for a 1D block of 128 threads on type int
  typedef cub::BlockReduce<T, Nthreads> BlockReduce;
  // Allocate shared memory for BlockReduce
  __shared__ typename BlockReduce::TempStorage temp_storage;

  T idata = 0;
  T xdata = 0;
  T ydata = 0;
  // load shared mem
  unsigned int tid = threadIdx.x;
  unsigned int xvalid = validx[blockIdx.x];
  unsigned int yvalid = validy[blockIdx.x];
  unsigned int x, y;
  int idim;

  for (int cc = 0; cc < nelem_thread; cc++) {
    x = ((tid * nelem_thread + cc) % npix);
    y = ((tid * nelem_thread + cc) / npix);
    // idim = tid * nelem_thread + cc + (blockDim.x * nelem_thread) *
    // blockIdx.x;
    idim = (x + xvalid) + (y + yvalid) * size;
    if (idim < size * size) {
      idata += (d_img[idim] > threshold) ? d_img[idim] : 0;
      xdata += (d_img[idim] > threshold) ? d_img[idim] * x : 0;
      ydata += (d_img[idim] > threshold) ? d_img[idim] * y : 0;
    }
  }

  // sdata[tid] = (i < N) ? g_idata[i] * x : 0;
  __syncthreads();

  T intensity = BlockReduce(temp_storage).Sum(idata, blockDim.x);
  T slopex = BlockReduce(temp_storage).Sum(xdata, blockDim.x);
  T slopey = BlockReduce(temp_storage).Sum(ydata, blockDim.x);
  // write result for this block to global mem
  if (tid == 0) {
    d_centroids[blockIdx.x] =
        ((slopex * 1.0 / (intensity + 1.e-6)) - offset) * scale -
        ref[blockIdx.x];
    d_centroids[blockIdx.x + gridDim.x] =
        ((slopey * 1.0 / (intensity + 1.e-6)) - offset) * scale -
        ref[blockIdx.x + gridDim.x];
    d_intensities[blockIdx.x] = intensity;
  }
}

template <class T>
void get_centroids(int size, int threads, int blocks, int npix, T *d_img,
                   T *d_centroids, T *ref, int *validx, int *validy,
                   T *intensities, T threshold, T scale, T offset,
                   carma_device *device) {
  int maxThreads = device->get_properties().maxThreadsPerBlock;
  unsigned int nelem_thread = 1;
  while ((threads / nelem_thread > maxThreads) ||
         (threads % nelem_thread != 0)) {
    nelem_thread++;
  }

  threads /= nelem_thread;
  dim3 dimBlock(threads, 1, 1);
  dim3 dimGrid(blocks, 1, 1);

  // when there is only one warp per block, we need to allocate two warps
  // worth of shared memory so that we don't index shared memory out of bounds
  if (threads <= 16)
    centroids<T, 16><<<dimGrid, dimBlock>>>(
        d_img, d_centroids, ref, validx, validy, intensities, threshold, npix,
        size, scale, offset, nelem_thread);
  else if (threads <= 32)
    centroids<T, 32><<<dimGrid, dimBlock>>>(
        d_img, d_centroids, ref, validx, validy, intensities, threshold, npix,
        size, scale, offset, nelem_thread);

  else if (threads <= 64)
    centroids<T, 64><<<dimGrid, dimBlock>>>(
        d_img, d_centroids, ref, validx, validy, intensities, threshold, npix,
        size, scale, offset, nelem_thread);
  else if (threads <= 128)
    centroids<T, 128><<<dimGrid, dimBlock>>>(
        d_img, d_centroids, ref, validx, validy, intensities, threshold, npix,
        size, scale, offset, nelem_thread);
  else if (threads <= 256)
    centroids<T, 256><<<dimGrid, dimBlock>>>(
        d_img, d_centroids, ref, validx, validy, intensities, threshold, npix,
        size, scale, offset, nelem_thread);
  else if (threads <= 512)
    centroids<T, 512><<<dimGrid, dimBlock>>>(
        d_img, d_centroids, ref, validx, validy, intensities, threshold, npix,
        size, scale, offset, nelem_thread);
  else
    printf("SH way too big !!!\n");

  carmaCheckMsg("centroids_kernel<<<>>> execution failed\n");
}

template void get_centroids<float>(int size, int threads, int blocks, int npix,
                                   float *d_img, float *d_centroids, float *ref,
                                   int *validx, int *validy, float *intensities,
                                   float threshold, float scale, float offset,
                                   carma_device *device);

template void get_centroids<double>(int size, int threads, int blocks, int npix,
                                    double *d_img, double *d_centroids,
                                    double *ref, int *validx, int *validy,
                                    double *intensities, double threshold,
                                    double scale, double offset,
                                    carma_device *device);
// template <class T>
// void get_centroids(int size, int threads, int blocks, int n, T *d_idata,
//                    T *d_odata, T *alpha, T thresh, T scale, T offset,
//                    carma_device *device) {
//   int maxThreads = device->get_properties().maxThreadsPerBlock;
//   unsigned int nelem_thread = 1;
//   while ((threads / nelem_thread > maxThreads) ||
//          (threads % nelem_thread != 0)) {
//     nelem_thread++;
//   }
//   threads /= nelem_thread;
//   dim3 dimBlock(threads, 1, 1);
//   dim3 dimGrid(blocks, 1, 1);

//   // when there is only one warp per block, we need to allocate two warps
//   // worth of shared memory so that we don't index shared memory out of
//   bounds int smemSize =
//       (threads <= 32) ? 2 * threads * sizeof(T) : threads * sizeof(T);
//   centroidx<T><<<dimGrid, dimBlock, smemSize>>>(
//       d_idata, d_odata, alpha, thresh, n, size, scale, offset, nelem_thread);

//   carmaCheckMsg("centroidx_kernel<<<>>> execution failed\n");

//   centroidy<T><<<dimGrid, dimBlock, smemSize>>>(d_idata, &(d_odata[blocks]),
//                                                 alpha, thresh, n, size,
//                                                 scale, offset, nelem_thread);

//   carmaCheckMsg("centroidy_kernel<<<>>> execution failed\n");
// }

// template void get_centroids<float>(int size, int threads, int blocks, int n,
//                                    float *d_idata, float *d_odata, float
//                                    *alpha,
//                                    float thresh, float scale, float offset,
//                                    carma_device *device);

// template void get_centroids<double>(int size, int threads, int blocks, int n,
//                                     double *d_idata, double *d_odata,
//                                     double *alpha, double thresh, double
//                                     scale,
//                                     double offset, carma_device *device);
// template <class T>
// __global__ void centroidx(T *g_idata, T *g_odata, T *alpha, T thresh,
//                           unsigned int n, unsigned int N, T scale, T offset,
//                           unsigned int nelem_thread) {
//   T *sdata = SharedMemory<T>();

//   // load shared mem
//   unsigned int tid = threadIdx.x;
//   // unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
//   // unsigned int x = (tid % n) + 1;
//   unsigned int x;
//   int idim;
//   sdata[tid] = 0;
//   for (int cc = 0; cc < nelem_thread; cc++) {
//     x = ((tid * nelem_thread + cc) % n);
//     idim = tid * nelem_thread + cc + (blockDim.x * nelem_thread) *
//     blockIdx.x; if (idim < N)
//       sdata[tid] += (g_idata[idim] > thresh) ? g_idata[idim] * x : 0;
//     else
//       sdata[tid] += 0;
//   }

//   // if (i < N)
//   //   sdata[tid] = (g_idata[i] > thresh) ? g_idata[i] * x : 0;

//   __syncthreads();

//   reduce_krnl(sdata, blockDim.x, tid);

//   // write result for this block to global mem
//   if (tid == 0)
//     g_odata[blockIdx.x] =
//         ((sdata[0] * 1.0 / (alpha[blockIdx.x] + 1.e-6)) - offset) * scale;
// }

// template <class T>
// __global__ void centroidy(T *g_idata, T *g_odata, T *alpha, T thresh,
//                           unsigned int n, unsigned int N, T scale, T offset,
//                           unsigned int nelem_thread) {
//   T *sdata = SharedMemory<T>();

//   // load shared mem
//   unsigned int tid = threadIdx.x;
//   // unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
//   // unsigned int y = (tid / n) + 1;
//   unsigned int y;
//   int idim;
//   sdata[tid] = 0;
//   for (int cc = 0; cc < nelem_thread; cc++) {
//     y = ((tid * nelem_thread + cc) / n);
//     idim = tid * nelem_thread + cc + (blockDim.x * nelem_thread) *
//     blockIdx.x; if (idim < N)
//       sdata[tid] += (g_idata[idim] > thresh) ? g_idata[idim] * y : 0;
//     else
//       sdata[tid] += 0;
//   }

//   // if (i < N)
//   //   sdata[tid] = (g_idata[i] > thresh) ? g_idata[i] * y : 0;

//   __syncthreads();

//   reduce_krnl(sdata, blockDim.x, tid);

//   // write result for this block to global mem
//   if (tid == 0)
//     g_odata[blockIdx.x] =
//         ((sdata[0] * 1.0 / (alpha[blockIdx.x] + 1.e-6)) - offset) * scale;
// }
