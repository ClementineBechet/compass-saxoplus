#include <carma_obj.h>
#include <convolutionFFT2D_common.h>

__global__ void fftconv_upadkrnl(float *odata, float *idata, int fftW, int dataW,int N, int n)
{
  __shared__ float cache[BLOCK_SZ][BLOCK_SZ];

  int x = threadIdx.x + blockIdx.x * blockDim.x;
  int y = threadIdx.y + blockIdx.y * blockDim.y;
  //int tid = x + y *blockDim.x * gridDim.x;

  if (y * fftW  + x < N) 
    cache[BLOCK_SZ-1-threadIdx.x][BLOCK_SZ-1-threadIdx.y] =  idata[y * fftW  + x];

  __syncthreads();

  if (y * dataW  + x < n)
    odata[y * dataW + x] = cache[BLOCK_SZ-1-threadIdx.x][BLOCK_SZ-1-threadIdx.y];
}


int fftconv_unpad_old(float *d_odata,float *d_idata,int fftW, int dataH, int dataW,int N, int n)
{

  dim3 blocks(dataH/BLOCK_SZ,dataW/BLOCK_SZ), threads(BLOCK_SZ,BLOCK_SZ);

  fftconv_upadkrnl<<<blocks, threads>>>(d_odata, d_idata, fftW,dataW,N,n);

   return EXIT_SUCCESS;
}

__global__ void unpad_krnl(float *odata, float *idata, int fftW, int dataW,int N, int n,int nim){
    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;
    const int z = blockDim.z * blockIdx.z + threadIdx.z;

    int kz_src = z*N;
    int kz_dst = z*n;

    if ((y * fftW  + x < N) && (z < nim)){
        odata[y * dataW + x + kz_dst] = idata[y * fftW  + x + kz_src];
    }
}

int fftconv_unpad(float *d_odata,float *d_idata,int fftW, int dataH, int dataW,int N, int n,int nim)
{
  dim3 threads(16,8, 8);
  dim3 grid(iDivUp(dataW, threads.x), iDivUp(dataH, threads.y), iDivUp(nim, threads.z));

  unpad_krnl<<<grid, threads>>>(d_odata, d_idata, fftW,dataW,N,n,nim);
  cutilCheckMsg("unpad_kernel<<<>>> execution failed\n");

   return EXIT_SUCCESS;
}