/**
 * \class carma_obj
 *
 * \ingroup libcarma
 *
 * \brief this class provides wrappers to the generic carma object
 *
 * \author $Author: dg, as $
 *
 * \version $Revision: 1.0 $
 *
 * \date $Date: 2011/01/28$
 *
 */
#ifndef _CARMA_OBJ_H_
#define _CARMA_OBJ_H_

#ifdef _USE_CUDPP
#include <cudpp.h>
#endif

#include <iostream>
#include <curand_kernel.h>
#include <curand.h>
#include <carma_utils.h>
#include <carma_streams.h>
#include <carma_context.h>

/*
create a memory object
void *memory
int  nb of reference

create a class which contains :
- d_data
- ndims
- dims
- strides
- type

new()

new(existing)


and then
modify carma_obj so that it is :
an object of the previous class
all the methods of a carma_obj

 */


#define BLOCK_SZ 16

using namespace std;
enum MemType { MT_DEVICE, MT_DARRAY, MT_HOST, MT_PAGELOCK, MT_ZEROCPY , MT_PORTABLE,
	       MT_WRICOMB, MT_GENEPIN }; //should add texture ?

template<class T_data>
class carma_data {

protected:
  T_data       *d_data;     ///< Pointer to data
  int          ndims;       ///< Number of dimensions
  int          nb_elem;     ///< Number of elements
  long         *dims_data;  ///< Dimensions
  int          *strides;    ///< Strides for each dimension
  MemType      malloc_type; ///< type of alloc

public:
  T_data  *get_data() { return d_data; }
  int     get_ndims() { return ndims; }
  int     get_nb_elem() { return nb_elem; }
  long    *get_dims_data() { return dims_data; }
  long    get_dims_data(int i) { return dims_data[i]; }
  int     *get_strides() { return strides; }
  int     get_strides(int i) { return strides[i]; }
  MemType get_malloc_type() {return malloc_type;}

};

template<class T_data> class carma_obj {

 protected:
  T_data *d_data;///< Input data  => change to vector
  T_data *o_data;///< optional data (used for scan / reduction)
  int ndim;
  long *dims_data;///< dimensions of the array
  int nb_elem;///< number of elements in the array
  int device; ///< device where the carma_obj is allocate
  carma_context *current_context;

  curandGenerator_t gen;
  curandState *d_states;

  int nThreads;
  int nBlocks;

#ifdef _USE_CUDPP
  CUDPPHandle   mScanPlan;        // CUDPP plan handle for prefix sum
#endif
  bool keysOnly; //< optional flag (used for sort)
  unsigned int *values;///< optional data (used for sort)
  size_t *d_numValid;///< used for compact

  cufftHandle plan;///< FFT plan
  cufftType tPlan;///< FFT plan type

  carma_streams *streams;

  void init(carma_context *current_context, long *dims_data, T_data *data, bool fromHost, int nb_streams);

 public:
  carma_obj(carma_obj<T_data> *obj);
  carma_obj(carma_context *current_context, long *dims_data);
  carma_obj(carma_context *current_context, carma_obj<T_data> *obj);
  carma_obj(carma_context *current_context, long *dims_data, T_data *data);
  carma_obj(carma_context *current_context, long *dims_data, int nb_streams);
  carma_obj(carma_context *current_context, carma_obj<T_data> *obj, int nb_streams);
  carma_obj(carma_context *current_context, long *dims_data, T_data *data, int nb_streams);
  ~carma_obj();

  int get_nbStreams();
  int add_stream();
  int add_stream(int nb);
  int del_stream();
  int del_stream(int nb);
  cudaStream_t get_cudaStream_t(int stream);
  int wait_stream(int stream);
  int wait_all_streams();

  /**< General Utilities */
  T_data* getData() { return d_data; }
  T_data* getOData() { return o_data; }
  long * getDims() { return dims_data; }
  long getDims(int i) { return dims_data[i]; }
  int getNbElem() { return nb_elem; }
  carma_context* getContext() { return current_context; }

  bool is_rng_init() { return (gen!=NULL); }


  /**< Memory transfers both ways */
  int host2device(T_data *data);
  int device2host(T_data *data);
  int device2hostOpt(T_data *data);
  int host2deviceVect(T_data *data,int incx, int incy);
  int device2hostVect(T_data *data,int incx,int incy);
  int host2deviceMat(T_data *data,int lda, int ldb);
  int device2hostMat(T_data *data,int lda, int ldb);

  int copyToDevice(T_data *data, int nb_elem);
  int copyFromDevice(T_data *data, int nb_elem);

  cufftHandle* getPlan() {return &plan;};///< FFT plan
  cufftType getTPlan()  {return tPlan;};///< FFT plan type

  unsigned int *getValues() {return values;};///< optional data (used for sort)

  /**< sum */
  T_data sum();

  /**< transpose */
  int transpose(carma_obj<T_data> *source);
  //carma_obj<T_data>& operator= (const carma_obj<T_data>& obj);

  /**< Cublas V2 */
  int imax(int incx);
  int imin(int incx);
  T_data asum(int incx);
  T_data nrm2(int incx);
  T_data dot(carma_obj<T_data> *source, int incx, int incy);
  void scale(T_data alpha, int incx);
  void swap(carma_obj<T_data> *source, int incx, int incy);
  void axpy(T_data alpha,carma_obj<T_data> *source, int incx, int incy);
  void rot(carma_obj<T_data> *source, int incx, int incy, T_data sc, T_data ss);

  void gemv(char trans, T_data alpha, carma_obj<T_data> *matA, int lda, carma_obj<T_data> *vectx, int incx, T_data beta, int incy);

  void ger(T_data alpha, carma_obj<T_data> *vectx, int incx, carma_obj<T_data> *vecty, int incy, int lda);

  void gemm(char transa, char transb, T_data alpha, carma_obj<T_data> *matA, int lda, carma_obj<T_data> *matB, int ldb, T_data beta, int ldc);

  /**< Curand */
  int init_prng(int device);
  int destroy_prng();
  int prng(T_data *output,char gtype, float alpha, float beta);
  int prng(T_data *output,char gtype, float alpha);
  int prng(char gtype, float alpha,float beta);
  int prng(char gtype, float alpha);
  int prng(char gtype);
  int init_prng_host(int seed);
  int prng_host(char gtype);
  int prng_host(char gtype, T_data alpha);
  int destroy_prng_host();

#ifdef _USE_CUDPP
  /**< cudpp */
  int sort_init(bool keysOnly);
  int sort();
  int config_scan(char *type, int dir, int incl);
  int destroy_scan();
  T_data scan();
  int host2deviceInd(unsigned int *ind);
  int device2hostInd(unsigned int *ind);
  int device2deviceInd(unsigned int *src);
  int compact_init();
  int compact(carma_obj<T_data> *dest);
  int compact(carma_obj<T_data> *dest,unsigned int *values);
#else
  /**< fake cudpp */
  int sort_init(bool keysOnly){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
  int sort(){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
  int config_scan(char *type, int dir, int incl){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
  int destroy_scan(){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
  T_data scan(){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
  int host2deviceInd(unsigned int *ind){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
  int device2hostInd(unsigned int *ind){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
  int device2deviceInd(unsigned int *src){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
  int compact_init(){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
  int compact(carma_obj<T_data> *dest){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
  int compact(carma_obj<T_data> *dest,unsigned int *values){
		cerr << "!!!!!! CUDPP not compiled !!!!!!"<<endl;
		return EXIT_SUCCESS;
	}
#endif

};

typedef carma_obj<int> caObjI;
typedef carma_obj<unsigned int> caObjUI;
typedef carma_obj<float> caObjS;
typedef carma_obj<double> caObjD;
typedef carma_obj<float2> caObjS2;
typedef carma_obj<double2> caObjD2;
typedef carma_obj<cuFloatComplex> caObjC;
typedef carma_obj<cuDoubleComplex> caObjZ;

  // CU functions sum
template <class T_data>
void reduce(int size, int threads, int blocks, T_data *d_idata, T_data *d_odata);

  // CU functions transpose
template<class T_data> int transposeCU(T_data *d_idata,T_data *d_odata,long N1,long N2);

  // CU functions generic
template<class T_data> int launch_generic1d(T_data *d_idata,T_data *d_odata,int N);
template<class T_data> int launch_generic2d(T_data *d_odata,T_data *d_idata,int N1, int N2);

  // CU functions curand
int carma_prng_init(int *seed,const int nThreads, const int nBlocks, curandState *state);
template <class T> int carma_prng_cu(T *results,const int nThreads,const int nBlocks,curandState *state,char gtype, int n, float alpha, float beta);

  // CU functions fft
template<class T_in, class T_out> cufftType carma_select_plan();
template <class T_in, class T_out> void carma_initfft(long *dims_data,cufftHandle plan,cufftType tPlan);
template <class T_in, class T_out> int carma_fft(T_in *input, T_out *output, int dir,cufftHandle plan);

  // CU functions generic
template<class T_data> int fillindex(T_data *d_odata,T_data *d_idata,int *indx,int N);
template<class T_data> int fillvalues(T_data *d_odata,unsigned int *indx,int N);
template<class T> int getarray2d(T *d_odata,T *d_idata,int x0, int Ncol,int NC, int N);
template<class T> int fillarray2d(T *d_odata,T *d_idata,int x0, int Ncol,int NC, int N);
template<class T> int fillarray2d2(T *d_odata,T *d_idata,int x0, int Ncol,int NC, int N);
template<class T> int carma_plus(T *d_odata,T elpha,int N);
template<class T> int carma_plusai(T *d_odata,T *i_data, int i,int sgn, int N);

  // CU functions fftconv
int fftconv_unpad(float *d_odata,float *d_idata,int fftW, int dataH, int dataW,int N, int n,int nim);
int carma_initfftconv(caObjS *data_in,caObjS *kernel_in,caObjS *padded_data,caObjC *padded_spectrum,int kernelY, int kernelX);
  // CPP functions fftconv
int carma_fftconv(caObjS *data_out,caObjS *padded_data, caObjC *padded_spectrum,int kernelY, int kernelX);

#ifdef _USE_MAGMA
  // MAGMA functions
template <class T> int carma_svd(carma_obj<T> *imat, carma_obj<T> *eigenvals, carma_obj<T> *mod2act, carma_obj<T> *mes2mod);
#else
template <class T> int carma_svd(carma_obj<T> *imat, carma_obj<T> *eigenvals, carma_obj<T> *mod2act, carma_obj<T> *mes2mod){
	cerr << "!!!!!! MAGMA not compiled !!!!!!"<<endl;
	return EXIT_SUCCESS;
}
#endif

// CULA functions
template <class T> int carma_cula_svd(carma_obj<T> *imat, carma_obj<T> *eigenvals, carma_obj<T> *mod2act, carma_obj<T> *mes2mod);
//

extern "C" {
  void sumGetNumBlocksAndThreads(int n,int device,int &blocks,int &threads);
  int snapTransformSize(int dataSize);
  bool isPow2(unsigned int x);
  unsigned int nextPow2( unsigned int x );
}

#endif // _CARMA_OBJ_H_