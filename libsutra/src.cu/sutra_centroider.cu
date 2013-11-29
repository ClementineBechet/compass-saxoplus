#include <sutra_centroider.h>

// Utility class used to avoid linker errors with extern
// unsized shared memory arrays with templated type
template<class T> struct SharedMemory
{
    __device__ inline operator       T*()
    {
        extern __shared__ int __smem[];
        return (T*)__smem;
    }

    __device__ inline operator const T*() const
    {
        extern __shared__ int __smem[];
        return (T*)__smem;
    }
};

// specialize for double to avoid unaligned memory 
// access compile errors
template<> struct SharedMemory<double>
{
    __device__ inline operator       double*()
    {
        extern __shared__ double __smem_d[];
        return (double*)__smem_d;
    }

    __device__ inline operator const double*() const
    {
        extern __shared__ double __smem_d[];
        return (double*)__smem_d;
    }
};


template <class T> 
__device__ inline void mswap(T & a, T & b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

/*
 _  __                    _     
| |/ /___ _ __ _ __   ___| |___ 
| ' // _ \ '__| '_ \ / _ \ / __|
| . \  __/ |  | | | |  __/ \__ \
|_|\_\___|_|  |_| |_|\___|_|___/
                                
 */

/*
    This version uses sequential addressing -- no divergence or bank conflicts.
*/
template <class T> __device__ void reduce_krnl(T *sdata, int size, int n)
{
  if (!((size&(size-1))==0)) {
    unsigned int s;
    if (size %2 != 0) s = size/2+1;
    else s = size/2;
    unsigned int s_old = size;
    while (s>0) {
      if ((n < s) && (n + s < s_old)) {
	sdata[n] += sdata[n + s];
      }
      __syncthreads();
      s_old = s;
      s /= 2;
      if ((2*s < s_old) && (s!=0)) s += 1;
    }
  } else {
    // do reduction in shared mem
    for(unsigned int s=size/2; s>0; s>>=1) {	
      if (n < s) {
	sdata[n] += sdata[n + s];
      }
      __syncthreads();
    }
  }
}


template <class T> __device__ void scanmax_krnl(T *sdata, int *values,int size, int n)
{
  if (!((size&(size-1))==0)) {
    unsigned int s;
    if (size %2 != 0) s = size/2+1;
    else s = size/2;
    unsigned int s_old = size;
    while (s>0) {
      if ((n < s) && (n + s < s_old)) {
	if (sdata[n] < sdata[n + s]) {
	  values[n] = n + s;
	  sdata[n] = sdata[n+s];
	}
      }
      __syncthreads();
      s_old = s;
      s /= 2;
      if ((2*s < s_old) && (s!=0)) s += 1;
    }
  } else {
    // do reduction in shared mem
    for(unsigned int s=size/2; s>0; s>>=1) {	
      if (n < s) {
	if (sdata[n] < sdata[n + s]) {
	  values[n] = n + s;
	  sdata[n] = sdata[n+s];
	}
      }
      __syncthreads();
    }
  }
}


template <class T> __device__ inline void sortmax_krnl(T *sdata, unsigned int *values,int size, int n)
{

  if (!((size&(size-1))==0)) {
    unsigned int s;
    if (size %2 != 0) s = size/2+1;
    else s = size/2;
    unsigned int s_old = size;
    while (s>0) {
      if ((n < s) && (n + s < s_old)) {
	if (sdata[n] < sdata[n + s]) {
	  mswap(values[n],values[n+s]);
	  mswap(sdata[n],sdata[n+s]);
	}
      }
      __syncthreads();
      s_old = s;
      s /= 2;
      if ((2*s < s_old) && (s!=0)) s += 1;
    }
  } else {
    // do reduction in shared mem
    for(unsigned int s=size/2; s>0; s>>=1) {	
      if (n < s) {
	if (sdata[n] < sdata[n + s]) {
	  mswap(values[n],values[n+s]);
	  mswap(sdata[n],sdata[n+s]);
	}
      }
      __syncthreads();
    }
  }
}

template <class T> __global__ void sortmax(T *g_idata, T *g_odata, int *values, int nmax)
{
  extern __shared__ uint svalues[];
  T *sdata = (T*)&svalues[blockDim.x];
  
  // load shared mem
  unsigned int tid = threadIdx.x;
  unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
  
  svalues[tid] = tid;
  sdata[tid]   = g_idata[i];
  
  __syncthreads();
  
  for (int cc=0;cc<nmax;cc++) {
    
    if (tid >= cc) sortmax_krnl(&(sdata[cc]),&(svalues[cc]),blockDim.x-cc,tid-cc);
    
    __syncthreads();
    
  } 
  
  if (tid < nmax) {
    g_odata[nmax*blockIdx.x+tid] = sdata[tid];
    values[nmax*blockIdx.x+tid]  = svalues[tid];
  }
  __syncthreads();
 
}

template <class T> __device__ inline void sortmaxi_krnl(T *sdata, unsigned int *values,int size, int n)
{

  if (!((size&(size-1))==0)) {
    unsigned int s;
    if (size %2 != 0) s = size/2+1;
    else s = size/2;
    unsigned int s_old = size;
    while (s>0) {
      if ((n < s) && (n + s < s_old)) {
	if (sdata[n] < sdata[n+s]) {
	  mswap(values[n],values[n+s]);
	  mswap(sdata[n],sdata[n+s]);
	}
      }
      __syncthreads();
      s_old = s;
      s /= 2;
      if ((2*s < s_old) && (s!=0)) s += 1;
    }
  } else {
    // do reduction in shared mem
    for(unsigned int s=size/2; s>0; s>>=1) {	
      if (n < s) {
	if (sdata[n] < sdata[n+s]) {
	  mswap(values[n],values[n+s]);
	  mswap(sdata[n],sdata[n+s]);
	}
      }
      __syncthreads();
    }
  }
}

template <class T> __global__ void sortmaxi(T *g_idata, int *values, int nmax,int offx, int offy, int npix, 
					    int Npix, int Npix2)
{
  extern __shared__ uint svalues[];
  T *sdata = (T*)&svalues[blockDim.x];
  
  // load shared mem
  unsigned int tid = threadIdx.x;
  
  svalues[tid] = tid;
  int nlig     = tid / npix;
  int ncol     = tid - nlig * npix;
  int idx      = offx + ncol + (nlig + offy) * Npix + blockIdx.x * Npix2;
  sdata[tid]   = g_idata[idx];
  
  __syncthreads();
  
  for (int cc=0;cc<nmax;cc++) {
    
    if (tid >= cc) sortmaxi_krnl(&(sdata[cc]),&(svalues[cc]),blockDim.x-cc,tid-cc);
    
    __syncthreads();
    
  } 
  
  if (tid < nmax) {
    nlig     = svalues[tid] / npix;
    ncol     = svalues[tid] - nlig * npix;
    idx      = offx + ncol + (nlig + offy) * Npix;
    values[nmax*blockIdx.x+tid]  = idx;
  }
  __syncthreads();
 
}

template <class T> __global__ void centroid_max(T *g_idata, T *g_odata, int n, int nmax, int nsub, T scale, T offset)
{
  extern __shared__ uint svalues[];
  T *sdata = (T*)&svalues[blockDim.x];
  T subsum;

  // load shared mem
  unsigned int tid = threadIdx.x;
  unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
  
  svalues[tid] = tid;
  sdata[tid]   = g_idata[i];
  
  __syncthreads();
  
  for (int cc=0;cc<nmax;cc++) {
    
    if (tid >= cc) sortmax_krnl(&(sdata[cc]),&(svalues[cc]),blockDim.x-cc,tid-cc);
    
    __syncthreads();
    
  } 
  // at this point the nmax first elements of sdata are the nmax brightest
  // pixels and the nmax first elements of svalues are their positions

  // first copy the brightest values out for reduction  
  if ((tid >= nmax) && (tid < 2*nmax)) sdata[tid] = sdata[tid-nmax];

  __syncthreads();

  reduce_krnl(sdata,nmax,tid);

  // get the sum per subap  
  if (tid == 0) subsum = sdata[tid];

  // put back the brightest pixels values 
  if ((tid >= nmax) && (tid < 2*nmax)) sdata[tid-nmax] = sdata[tid];

  __syncthreads();

  // compute the centroid on the first part of the array
  if (tid < nmax) sdata[tid] *= ((svalues[tid] % n) + 1);
  // x centroid
  __syncthreads();
  reduce_krnl(sdata,nmax,tid);

  if (tid == 0) g_odata[blockIdx.x] = sdata[tid]/subsum;

  // put back the brightest pixels values 
  if ((tid >= nmax) && (tid < 2*nmax)) sdata[tid-nmax] = sdata[tid];

  __syncthreads();

  // compute the centroid on the first part of the array
  if (tid < nmax) sdata[tid] *= (svalues[tid] / n + 1);
  // y centroid
  __syncthreads();
  reduce_krnl(sdata,nmax,tid);

  if (tid == 0) g_odata[blockIdx.x+nsub] = ((sdata[tid]/subsum) - offset) * scale;
 
}


template <class T> __global__ void centroidx(T *g_idata, T *g_odata, T *alpha, unsigned int n, unsigned int N, T scale, T offset)
{
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
    
    sdata[tid] = (i < N) ? g_idata[i] * ((tid % n) +1) : 0;

    __syncthreads();

    reduce_krnl(sdata,blockDim.x,tid);

    // write result for this block to global mem
    if (tid == 0) g_odata[blockIdx.x] = ((sdata[0]*1.0/(alpha[blockIdx.x]+1.e-6)) - offset) * scale;
}

template <class T> __global__ void centroidy(T *g_idata, T *g_odata, T *alpha, unsigned int n, unsigned int N, T scale, T offset)
{
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
    
    sdata[tid] = (i < N) ? g_idata[i] * ((tid / n) +1) : 0;

    __syncthreads();

    reduce_krnl(sdata,blockDim.x,tid);

    // write result for this block to global mem
    if (tid == 0) g_odata[blockIdx.x] = ((sdata[0]*1.0/(alpha[blockIdx.x]+1.e-6)) - offset) * scale;
}

template <class T> __global__ void centroidx_async(T *g_idata, T *g_odata, T *alpha, unsigned int n, unsigned int N, T scale, T offset, int stream_offset)
{
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
    i+=stream_offset*blockDim.x;

    sdata[tid] = (i < N) ? g_idata[i] * ((tid % n) +1) : 0;

    __syncthreads();

    reduce_krnl(sdata,blockDim.x,tid);

    // write result for this block to global mem
    if (tid == 0) g_odata[blockIdx.x+stream_offset] = ((sdata[0]*1.0/(alpha[blockIdx.x+stream_offset]+1.e-6)) - offset) * scale;
}

template <class T> __global__ void centroidy_async(T *g_idata, T *g_odata, T *alpha, unsigned int n, unsigned int N, T scale, T offset, int stream_offset)
{
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
    i+=stream_offset*blockDim.x;

    sdata[tid] = (i < N) ? g_idata[i] * ((tid / n) +1) : 0;

    __syncthreads();

    reduce_krnl(sdata,blockDim.x,tid);

    // write result for this block to global mem
    if (tid == 0) g_odata[blockIdx.x+stream_offset] = ((sdata[0]*1.0/(alpha[blockIdx.x+stream_offset]+1.e-6)) - offset) * scale;
}

template <class T> void get_centroids_async(int threads, int blocks, int n, carma_streams *streams, T *d_idata, T *d_odata,  T *alpha, T scale, T offset)
{
  int nstreams = streams->get_nbStreams();
  int nbelem = threads * blocks;
  
  dim3 dimBlock(threads);
  dim3 dimGrid(blocks/nstreams);
  
  // when there is only one warp per block, we need to allocate two warps
  // worth of shared memory so that we don't index shared memory out of bounds
  int smemSize = (threads <= 32) ? 2 * threads * sizeof(T) : threads * sizeof(T);
  for(int i=0; i<nstreams; i++) {
    centroidx_async<T><<< dimGrid, dimBlock, smemSize, streams->get_stream(i) >>>(d_idata, d_odata, alpha, n, nbelem,scale,offset,i*blocks/nstreams);
    
    cutilCheckMsg("centroidx_kernel<<<>>> execution failed\n");
    
    centroidy_async<T><<< dimGrid, dimBlock, smemSize, streams->get_stream(i) >>>(d_idata, &(d_odata[blocks]), alpha, n, nbelem,scale,offset,i*blocks/nstreams);
    
    cutilCheckMsg("centroidy_kernel<<<>>> execution failed\n");
  }
}

template void get_centroids_async<float>(int threads, int blocks, int n, carma_streams *streams, float *d_idata, float *d_odata, float *alpha, float scale, float offset);

template <class T> __global__ void centroidx(T *g_idata, T *g_odata, T *alpha, T thresh, unsigned int n, unsigned int N, T scale, T offset)
{
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
    
    if (i < N) sdata[tid] = (g_idata[i] > thresh) ? g_idata[i] * ((tid % n) +1) : 0;

    __syncthreads();

    reduce_krnl(sdata,blockDim.x,tid);

    // write result for this block to global mem
    if (tid == 0) g_odata[blockIdx.x] = ((sdata[0]*1.0/(alpha[blockIdx.x]+1.e-6)) - offset) * scale;
}

template <class T> __global__ void centroidy(T *g_idata, T *g_odata, T *alpha, T thresh, unsigned int n, unsigned int N, T scale, T offset)
{
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
    
    if (i < N) sdata[tid] = (g_idata[i] > thresh) ? g_idata[i] * ((tid / n) +1) : 0;

    __syncthreads();

    reduce_krnl(sdata,blockDim.x,tid);

    // write result for this block to global mem
    if (tid == 0) g_odata[blockIdx.x] = ((sdata[0]*1.0/(alpha[blockIdx.x]+1.e-6)) - offset) * scale;
}

template <class T> __global__ void centroidx(T *g_idata, T *g_odata, T *alpha, T *weights, unsigned int n, unsigned int N, T scale, T offset)
{
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
    
    sdata[tid] = (i < N) ? g_idata[i] * ((tid % n) +1) * weights[i] : 0;

    __syncthreads();

    reduce_krnl(sdata,blockDim.x,tid);

    // write result for this block to global mem
    if (tid == 0) g_odata[blockIdx.x] = ((sdata[0]*1.0/(alpha[blockIdx.x]+1.e-6)) - offset) * scale;
}

template <class T> __global__ void centroidy(T *g_idata, T *g_odata, T *alpha, T *weights, unsigned int n, unsigned int N, T scale, T offset)
{
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
    
    sdata[tid] = (i < N) ? g_idata[i] * ((tid / n) +1) * weights[i] : 0;

    __syncthreads();

    reduce_krnl(sdata,blockDim.x,tid);

    // write result for this block to global mem
    if (tid == 0) g_odata[blockIdx.x] = ((sdata[0]*1.0/(alpha[blockIdx.x]+1.e-6)) - offset) * scale;
}


template <class T> __global__ void interp_parab(T *g_idata,T *g_centroids, int *g_values, T *g_matinterp,
						int sizex, int sizey, int nvalid, int Npix, int Npix2, T scale, T offset)
{
  extern __shared__ T pidata[];
  T *scoeff   = (T*)&pidata[blockDim.x];
  T *m_interp = (T*)&scoeff[6];
  
  int offy = g_values[blockIdx.x] / Npix;
  int offx = g_values[blockIdx.x] - offy * Npix;
  offx     -=  sizex / 2;
  offy     -=  sizey / 2;
  int nlig = threadIdx.x / sizex;
  int ncol = threadIdx.x - nlig * sizex;
  int idx  = offx + ncol + (nlig + offy) * Npix + blockIdx.x * Npix2;
  
  // load shared mem
  pidata[threadIdx.x]   = g_idata[idx];

  __syncthreads();

  for (int cc=0;cc<6;cc++) m_interp[cc * sizex * sizey + threadIdx.x] = 
			     g_matinterp[cc * sizex * sizey + threadIdx.x] * pidata[threadIdx.x];

  __syncthreads();

  // do reduction for each 6 coeffs
  if (threadIdx.x < 6) {
    scoeff[threadIdx.x] = 0.0f;
    for (int cc=0;cc<sizex*sizey;cc++) scoeff[threadIdx.x] += m_interp[cc + threadIdx.x * sizex * sizey];
  }
  
  __syncthreads();

  // now retreive x0 and y0 from scoeff
  if (threadIdx.x < 2) {
    T denom = scoeff[2] * scoeff[2] - 4.0f * scoeff[1] * scoeff[0];
    if (denom == 0) {
      if (threadIdx.x == 0) 
	g_centroids[blockIdx.x] = 0.0f;
      if (threadIdx.x == 1) 
	g_centroids[blockIdx.x+nvalid] = 0.0f;
    } else {
      if (threadIdx.x == 0) {
	g_centroids[blockIdx.x] = (2.0f * scoeff[1] * scoeff[3] - scoeff[4] * scoeff[2]) / denom;
	int xm = (2*offx + sizex);
	xm = (xm % 2 == 0) ? xm/2 : xm/2 +1;
	g_centroids[blockIdx.x] += (xm + 0.5 - (Npix+1)/4);
	g_centroids[blockIdx.x] = (g_centroids[blockIdx.x] - offset) * scale;
      } 
      if (threadIdx.x == 1) {
	g_centroids[blockIdx.x+nvalid] = (2.0f * scoeff[0] * scoeff[4] - scoeff[3] * scoeff[2]) / denom;
	int ym = (2*offy + sizey);
	ym = (ym % 2 == 0) ? ym/2 : ym/2 +1;
	g_centroids[blockIdx.x+nvalid] += (ym + 0.5 - (Npix+1)/4);
	g_centroids[blockIdx.x+nvalid] = (g_centroids[blockIdx.x+nvalid] - offset) * scale;
     }
    }
  }

  __syncthreads();
  /*
  if (threadIdx.x == 0) g_centroids[blockIdx.x] = (g_centroids[blockIdx.x] - offset) * scale;
  if (threadIdx.x == 1) g_centroids[blockIdx.x+nvalid] = (g_centroids[blockIdx.x+nvalid] - offset) * scale;
  */
}


__global__ void fillweights_krnl(float *d_out, float *weights,int Npix,int N)
{
  int nim,idx;
  int tid   = threadIdx.x + blockIdx.x * blockDim.x;

  while (tid < N) {
    nim = tid / Npix;
    idx = tid - nim * Npix;
    d_out[tid] = weights[idx];
    tid  += blockDim.x * gridDim.x;
  }
}

__global__ void fillcorrcube_krnl(cuFloatComplex *d_out, float *d_in,int npix_in, int Npix_in, int npix_out, int Npix_out, int N)
{
  int nim,npix,nlig,ncol,idx;
  int tid   = threadIdx.x + blockIdx.x * blockDim.x;

  while (tid < N) {
    nim = tid / Npix_in;
    npix = tid - nim * Npix_in;
    nlig = npix / npix_in;
    ncol = npix - nlig * npix_in;
    idx = nlig * npix_out + ncol + nim * Npix_out;
    d_out[idx].x = d_in[tid];
    d_out[idx].y = 0.0;
    tid  += blockDim.x * gridDim.x;
  }
}

__global__ void fillcorrim_krnl(cuFloatComplex *d_out, float *d_in,int npix_in, int Npix_in, int npix_out, int Npix_out, int N)
{
  int nim,npix,nlig,ncol,idx;
  int tid   = threadIdx.x + blockIdx.x * blockDim.x;

  while (tid < N) {
    nim = tid / Npix_in;
    npix = tid - nim * Npix_in;
    nlig = npix / npix_in;
    ncol = npix - nlig * npix_in;
    idx = nlig * npix_out + ncol + nim * Npix_out;
    d_out[idx].x = d_in[npix];
    d_out[idx].y = 0.0;
    tid  += blockDim.x * gridDim.x;
  }
}

__global__ void fillval_krnl(cuFloatComplex *d_out, float val,int npix_in,int npix_out,int N)
{
  int nim,npix,idx;
  int tid   = threadIdx.x + blockIdx.x * blockDim.x;

  while (tid < N) {
    nim = tid / npix_in;
    npix = tid % npix_in;
    idx = nim * npix_out + npix;
    d_out[idx].x = val;
    d_out[idx].y = 0.0;
    tid  += blockDim.x * gridDim.x;
  }
}

__global__ void corr_krnl(cuFloatComplex *odata,cuFloatComplex *idata, int N)
{

  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  __shared__ cuFloatComplex tmp;

  while (tid < N) {
    tmp.x = idata[tid].x*odata[tid].x+idata[tid].y*odata[tid].y;
    tmp.y = -1.0f*idata[tid].y*odata[tid].x+idata[tid].x*odata[tid].y;
    odata[tid].x = tmp.x;
    odata[tid].y = tmp.y;
    tid += blockDim.x * gridDim.x;
  }
}

__global__ void roll2real_krnl(float *odata,cuFloatComplex *idata, int n, int Npix, int N)
{
  // here we need to roll and keep only the (2:,2:,) elements
  // startegy is to go through all elements of input array
  // if final index > 0 (in x & y) keep it in output with (idx-1,idy-1)
  // n is always odd because it is 2 x npix
  int nim,idpix,idx,idy,idt;
  int tid = threadIdx.x + blockIdx.x * blockDim.x;

  while (tid < N) {
    nim = tid / Npix;
    idpix = tid - nim * Npix;
    idy = idpix / n;
    idx = idpix - n * idy;

    idx = (idx + n/2) % n;
    idy = (idy + n/2) % n;

    if ((idx > 0) && (idy > 0)) {
      idt = idx - 1 + (idy - 1) * (n - 1) + nim * (n - 1) * (n - 1); 
      odata[idt] = idata[tid].x;
    }
    tid += blockDim.x * gridDim.x;
  }
}

__global__ void corrnorm_krnl(float *odata,float *idata, int Npix, int N)
{

  int nim,idpix;
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  __shared__ float tmp;

  while (tid < N) {
    nim = tid / Npix;
    idpix = tid - nim * Npix;
    tmp = odata[tid] / idata[idpix];
    odata[tid] = tmp; 
    tid += blockDim.x * gridDim.x;
  }
}

__global__ void convert_krnl(float *odata,float *idata, float offset, float scale, int N)
{

  int tid = threadIdx.x + blockIdx.x * blockDim.x;

  while (tid < N) {
    odata[tid] = (idata[tid]-offset)*scale; 
    tid += blockDim.x * gridDim.x;
  }
}

/*
 _                           _                   
| |    __ _ _   _ _ __   ___| |__   ___ _ __ ___ 
| |   / _` | | | | '_ \ / __| '_ \ / _ \ '__/ __|
| |__| (_| | |_| | | | | (__| | | |  __/ |  \__ \
|_____\__,_|\__,_|_| |_|\___|_| |_|\___|_|  |___/
                                                 
 */


template <class T> void subap_sortmax(int size, int threads, int blocks, T *d_idata, T *d_odata, int *values, int nmax)
{
    dim3 dimBlock(threads, 1, 1);
    dim3 dimGrid(blocks, 1, 1);

    // when there is only one warp per block, we need to allocate two warps 
    // worth of shared memory so that we don't index shared memory out of bounds
    int smemSize = (threads <= 32) ? 2 * threads * (sizeof(T) + sizeof(uint)) : threads * (sizeof(T) + sizeof(uint));
    sortmax<T><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, values, nmax);

    cutilCheckMsg("sortmax_kernel<<<>>> execution failed\n");
}
template void subap_sortmax<float>(int size, int threads, int blocks, float *d_idata, float *d_odata, int *values, int nmax);

template void subap_sortmax<double>(int size, int threads, int blocks, double *d_idata, double *d_odata, int *values, int nmax);


template <class T> void subap_centromax(int threads, int blocks, T *d_idata, T *d_odata, int npix, int nmax, T scale, T offset)
{
    dim3 dimBlock(threads, 1, 1);
    dim3 dimGrid(blocks, 1, 1);

    // when there is only one warp per block, we need to allocate two warps 
    // worth of shared memory so that we don't index shared memory out of bounds
    int smemSize = (threads <= 32) ? 2 * threads * (sizeof(T) + sizeof(uint)) : threads * (sizeof(T) + sizeof(uint));
    centroid_max<T><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata,npix, nmax,blocks,scale,offset);

    cutilCheckMsg("centroid_kernel<<<>>> execution failed\n");
}
template void subap_centromax<float>( int threads, int blocks, float *d_idata, float *d_odata, int npix, int nmax, float scale, float offset);

template void subap_centromax<double>(int threads, int blocks, double *d_idata, double *d_odata, int npix, int nmax, double scale, double offset);


template <class T> void get_centroids(int size, int threads, int blocks, int n, T *d_idata, T *d_odata, T *alpha, T scale, T offset)
{
    dim3 dimBlock(threads, 1, 1);
    dim3 dimGrid(blocks, 1, 1);

    // when there is only one warp per block, we need to allocate two warps 
    // worth of shared memory so that we don't index shared memory out of bounds
    int smemSize = (threads <= 32) ? 2 * threads * sizeof(T) : threads * sizeof(T);
    centroidx<T><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, alpha, n, size,scale,offset);

    cutilCheckMsg("centroidx_kernel<<<>>> execution failed\n");

    centroidy<T><<< dimGrid, dimBlock, smemSize >>>(d_idata, &(d_odata[blocks]), alpha, n, size,scale,offset);

    cutilCheckMsg("centroidy_kernel<<<>>> execution failed\n");
}

template void get_centroids<float>(int size, int threads, int blocks, int n,float *d_idata, float *d_odata,float *alpha, float scale, float offset);

template void get_centroids<double>(int size, int threads, int blocks, int n,double *d_idata, double *d_odata, double *alpha, double scale, double offset);


template <class T> void get_centroids(int size, int threads, int blocks, int n, T *d_idata, T *d_odata, T *alpha, T thresh, T scale, T offset)
{
    dim3 dimBlock(threads, 1, 1);
    dim3 dimGrid(blocks, 1, 1);

    // when there is only one warp per block, we need to allocate two warps 
    // worth of shared memory so that we don't index shared memory out of bounds
    int smemSize = (threads <= 32) ? 2 * threads * sizeof(T) : threads * sizeof(T);
    centroidx<T><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, alpha, thresh, n, size,scale,offset);

    cutilCheckMsg("centroidx_kernel<<<>>> execution failed\n");

    centroidy<T><<< dimGrid, dimBlock, smemSize >>>(d_idata, &(d_odata[blocks]), alpha, thresh, n, size,scale,offset);

    cutilCheckMsg("centroidy_kernel<<<>>> execution failed\n");
}

template void get_centroids<float>(int size, int threads, int blocks, int n,float *d_idata, float *d_odata,float *alpha, float thresh, float scale, float offset);

template void get_centroids<double>(int size, int threads, int blocks, int n,double *d_idata, double *d_odata, double *alpha, double thresh, double scale, double offset);


template <class T> void get_centroids(int size, int threads, int blocks, int n, T *d_idata, T *d_odata, T *alpha, T *weights, T scale, T offset)
{
    dim3 dimBlock(threads, 1, 1);
    dim3 dimGrid(blocks, 1, 1);

    // when there is only one warp per block, we need to allocate two warps 
    // worth of shared memory so that we don't index shared memory out of bounds
    int smemSize = (threads <= 32) ? 2 * threads * sizeof(T) : threads * sizeof(T);
    centroidx<T><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, alpha, weights, n, size,scale,offset);

    cutilCheckMsg("centroidx_kernel<<<>>> execution failed\n");

    centroidy<T><<< dimGrid, dimBlock, smemSize >>>(d_idata, &(d_odata[blocks]), alpha, weights, n, size,scale,offset);

    cutilCheckMsg("centroidy_kernel<<<>>> execution failed\n");
}

template void get_centroids<float>(int size, int threads, int blocks, int n,float *d_idata, float *d_odata,float *alpha, float *weights, float scale, float offset);

template void get_centroids<double>(int size, int threads, int blocks, int n,double *d_idata, double *d_odata, double *alpha, double *weights, double scale, double offset);



int fillweights(float *d_out, float *d_in, int npix, int N, int device)
{

  struct cudaDeviceProp deviceProperties;
  cudaGetDeviceProperties(&deviceProperties, device);
  
  int maxThreads = deviceProperties.maxThreadsPerBlock;
  int nBlocks = deviceProperties.multiProcessorCount*8;
  int nThreads = (N + nBlocks -1)/nBlocks;

  if (nThreads > maxThreads) {
    nThreads = maxThreads;
    nBlocks = (N + nThreads  -1)/nThreads;
  }

  dim3 grid(nBlocks), threads(nThreads);

  fillweights_krnl<<<grid, threads>>>(d_out,d_in,npix*npix,N);
  cutilCheckMsg("<<<fillweights_krnl>>> execution failed\n");

  return EXIT_SUCCESS;
}

int fillcorr(cuFloatComplex *d_out, float *d_in, int npix_in, int npix_out, int N, int nvalid, int device)
{

  struct cudaDeviceProp deviceProperties;
  cudaGetDeviceProperties(&deviceProperties, device);
  
  int maxThreads = deviceProperties.maxThreadsPerBlock;
  int nBlocks = deviceProperties.multiProcessorCount*8;
  int nThreads = (N + nBlocks -1)/nBlocks;

  if (nThreads > maxThreads) {
    nThreads = maxThreads;
    nBlocks = (N + nThreads  -1)/nThreads;
  }

  dim3 grid(nBlocks), threads(nThreads);

  if (nvalid == 1) {
    //cout << "3d" << endl;
    fillcorrcube_krnl<<<grid, threads>>>(d_out,d_in,npix_in,npix_in*npix_in,npix_out,npix_out*npix_out,N);
  } else { 
    fillcorrim_krnl<<<grid, threads>>>(d_out,d_in,npix_in,npix_in*npix_in,npix_out,npix_out*npix_out,N);
    //cout << "2d" << endl;
  }

  cutilCheckMsg("fillcorr_kernel<<<>>> execution failed\n");

  return EXIT_SUCCESS;
}

int correl(cuFloatComplex *d_odata,cuFloatComplex *d_idata,int N,int device)
{

  struct cudaDeviceProp deviceProperties;
  cudaGetDeviceProperties(&deviceProperties, device);
    
  int maxThreads = deviceProperties.maxThreadsPerBlock;
  int nBlocks = deviceProperties.multiProcessorCount*8;
  int nThreads = (N + nBlocks -1)/nBlocks;

  if (nThreads > maxThreads) {
    nThreads = maxThreads;
    nBlocks = (N + nThreads  -1)/nThreads;
  }

  dim3 grid(nBlocks), threads(nThreads);

  corr_krnl<<<grid, threads>>>(d_odata, d_idata, N);

   return EXIT_SUCCESS;
}

int roll2real(float *d_odata,cuFloatComplex *d_idata, int n, int Npix,  int N, int device)
{

  struct cudaDeviceProp deviceProperties;
  cudaGetDeviceProperties(&deviceProperties, device);
    
  int maxThreads = deviceProperties.maxThreadsPerBlock;
  int nBlocks = deviceProperties.multiProcessorCount*8;
  int nThreads = (N + nBlocks -1)/nBlocks;

  if (nThreads > maxThreads) {
    nThreads = maxThreads;
    nBlocks = (N + nThreads  -1)/nThreads;
  }

  dim3 grid(nBlocks), threads(nThreads);

  roll2real_krnl<<<grid, threads>>>(d_odata, d_idata, n, Npix, N);

   return EXIT_SUCCESS;
}

int corr_norm(float *d_odata,float *d_idata,int Npix, int N,int device)
{

  struct cudaDeviceProp deviceProperties;
  cudaGetDeviceProperties(&deviceProperties, device);
    
  int maxThreads = deviceProperties.maxThreadsPerBlock;
  int nBlocks = deviceProperties.multiProcessorCount*8;
  int nThreads = (N + nBlocks -1)/nBlocks;

  if (nThreads > maxThreads) {
    nThreads = maxThreads;
    nBlocks = (N + nThreads  -1)/nThreads;
  }

  dim3 grid(nBlocks), threads(nThreads);

  corrnorm_krnl<<<grid, threads>>>(d_odata, d_idata, Npix, N);

   return EXIT_SUCCESS;
}


int fillval_corr(cuFloatComplex *d_out, float val, int npix_in, int npix_out, int N, int device)
{

  struct cudaDeviceProp deviceProperties;
  cudaGetDeviceProperties(&deviceProperties, device);
  
  int maxThreads = deviceProperties.maxThreadsPerBlock;
  int nBlocks = deviceProperties.multiProcessorCount*8;
  int nThreads = (N + nBlocks -1)/nBlocks;

  if (nThreads > maxThreads) {
    nThreads = maxThreads;
    nBlocks = (N + nThreads  -1)/nThreads;
  }

  dim3 grid(nBlocks), threads(nThreads);

  fillval_krnl<<<grid, threads>>>(d_out,val,npix_in,npix_out,N);
  cutilCheckMsg("fillcorr_kernel<<<>>> execution failed\n");

  return EXIT_SUCCESS;
}

template <class T> void subap_sortmaxi(int threads, int blocks, T *d_idata,  int *values, int nmax,
				       int offx, int offy, int npix, int Npix)
// here idata is a [Npix,Npix,nvalid] array
// we want to get the [nvalid] max into subregions of [npix,npix] starting at [xoff,yoff]
// number of threads is npix * npix and number of blocks : nvalid
{
    dim3 dimBlock(threads, 1, 1);
    dim3 dimGrid(blocks, 1, 1);

    // when there is only one warp per block, we need to allocate two warps 
    // worth of shared memory so that we don't index shared memory out of bounds
    int smemSize = (threads <= 32) ? 2 * threads * (sizeof(T) + sizeof(uint)) : threads * (sizeof(T) + sizeof(uint));
    sortmaxi<T><<< dimGrid, dimBlock, smemSize >>>(d_idata, values,nmax,offx,offy,npix,Npix,Npix*Npix);

    cutilCheckMsg("sortmaxi_kernel<<<>>> execution failed\n");
}
template void subap_sortmaxi<float>(int threads, int blocks, float *d_idata, int *values, int nmax,
				       int offx, int offy, int npix, int Npix);

template void subap_sortmaxi<double>(int threads, int blocks, double *d_idata, int *values, int nmax,
				       int offx, int offy, int npix, int Npix);


/*
algorithm for parabolic interpolation
we do the interpolation on nx x ny points
we use a (nx * ny, 6) interp matrix
we thus need nx * ny arrays in shared mem this is the number of threads
we have nvalid blocks
*/

template <class T> void subap_pinterp(int threads, int blocks, T *d_idata,  int *values, T *d_centroids,
				      T *d_matinterp, int sizex,int sizey, int nvalid, int Npix, T scale, T offset)
// here idata is a [Npix,Npix,nvalid] array
// we want to get the [nvalid] (x0,y0) into subregions of [sizex,sizey] around gvalue
// number of threads is sizex * sizey  and number of blocks : nvalid
{
    dim3 dimBlock(threads, 1, 1);
    dim3 dimGrid(blocks, 1, 1);

    int shsize = (threads + threads * 6 + 6);
    // when there is only one warp per block, we need to allocate two warps 
    // worth of shared memory so that we don't index shared memory out of bounds

    int smemSize = (shsize <= 32) ? 2 * shsize * sizeof(T) : shsize * sizeof(T);
    interp_parab<T><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_centroids, values,d_matinterp,sizex,sizey,
						       nvalid,Npix,Npix*Npix,scale,offset);

    cutilCheckMsg("sortmaxi_kernel<<<>>> execution failed\n");
}

template void subap_pinterp<float>(int threads, int blocks, float *d_idata,  int *values, float *d_centroids,
				   float *d_matinterp, int sizex,int sizey, int nvalid, int Npix, float scale, float offset);
/*
template void subap_pinterp<double>(int threads, int blocks, double *d_idata,  int *values, double *d_centroids,
				    double *d_matinterp, int sizex,int sizey, int nvalid, int Npix);
*/

int convert_centro(float *d_odata,float *d_idata,float offset, float scale, int N,int device)
{

  struct cudaDeviceProp deviceProperties;
  cudaGetDeviceProperties(&deviceProperties, device);
    
  int maxThreads = deviceProperties.maxThreadsPerBlock;
  int nBlocks = deviceProperties.multiProcessorCount*8;
  int nThreads = (N + nBlocks -1)/nBlocks;

  if (nThreads > maxThreads) {
    nThreads = maxThreads;
    nBlocks = (N + nThreads  -1)/nThreads;
  }

  dim3 grid(nBlocks), threads(nThreads);

  convert_krnl<<<grid, threads>>>(d_odata, d_idata, offset, scale, N);

   return EXIT_SUCCESS;
}


template <class T> __global__ void pyrslopes_krnl(T *g_odata, T *g_idata, int *subindx, int *subindy, T *subsum, unsigned int ns, unsigned int nvalid, unsigned int nim)
{
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;

    if (i < nvalid) {
      int i2 = subindx[i] +  subindy[i] * ns;
      g_odata[i]        = ((g_idata[i2+ns*ns] + g_idata[i2+3*ns*ns]) - (g_idata[i2] + g_idata[i2+2*ns*ns])) / subsum[i];
      g_odata[i+nvalid] = ((g_idata[i2+2*ns*ns] + g_idata[i2+3*ns*ns]) - (g_idata[i2] + g_idata[i2+ns*ns])) / subsum[i];
    }
}

template <class T> void pyr_slopes(T *d_odata, T *d_idata, int *subindx, int *subindy, T *subsum, int ns, int nvalid, int nim, int device)
{
  //cout << "hello cu" << endl;
  struct cudaDeviceProp deviceProperties;
  cudaGetDeviceProperties(&deviceProperties, device);
  
  int N = nvalid;

  int maxThreads = deviceProperties.maxThreadsPerBlock;
  int nBlocks = deviceProperties.multiProcessorCount*8;
  int nThreads = (N + nBlocks -1)/nBlocks;

  if (nThreads > maxThreads) {
    nThreads = maxThreads;
    nBlocks = (N + nThreads  -1)/nThreads;
  }

  dim3 grid(nBlocks), threads(nThreads);

  pyrslopes_krnl<T><<< grid, threads >>>(d_odata, d_idata,subindx,subindy,subsum,ns,nvalid,nim);

  cutilCheckMsg("pyrslopes_kernel<<<>>> execution failed\n");
}

template void pyr_slopes<float>(float *d_odata, float *d_idata, int *subindx, int *subindy, float *subsum, int ns, int nvalid, int nim, int device);
template void pyr_slopes<double>(double *d_odata, double  *d_idata, int *subindx, int *subindy, double *subsum, int ns, int nvalid, int nim, int device);

