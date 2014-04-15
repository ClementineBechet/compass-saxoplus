#ifndef _sutra_controller_H_
#define _sutra_controller_H_

#include <carma_cublas.h>
#include <carma_host_obj.h>
#include <sutra_wfs.h>
#include <sutra_dm.h>
#include <sutra_centroider.h>
#include <sutra_ao_utils.h>
#include <cured.h>

using namespace std;

class sutra_controller {
public:

  //allocation of d_centroids and d_com
  sutra_controller(carma_context* context, int nslope, int nactu);
  virtual
  ~sutra_controller();

  virtual string
  get_type()=0;

  //!!!! YOU MUST set d_centroids before call it!!!!
  virtual int
  comp_com()=0;

  //It is better to have something like this (+protected d_centroids):
  //virtual int comp_com (carma_obj<float> *new_centroids)=0;
  //it would imply copy, but would be much safer

  int nactu() {
    return d_com->getDims(1);
  }
  int nslope() {
    return d_centroids->getDims(1);
  }

  cublasHandle_t cublas_handle() {
    return current_context->get_cublasHandle();
  }

public:
//I would propose to make them protected (+ proper
//set of fuctions). It could make life easier!
//But we should discuss it
  carma_obj<float> *d_centroids; // current centroids
  carma_obj<float> *d_com; // current command

protected:
  int device;
  carma_context *current_context;

};

int
shift_buf(float *d_data, int offset, int N, int device);
int
mult_vect(float *d_data, float *scale, int N, int device);
int
mult_int(float *o_data, float *i_data, float *scale, float gain, int N,
    int device);
int
mult_int(float *o_data, float *i_data, float *scale, float gain, int N,
    int device, carma_streams *streams);
int
fill_filtmat(float *filter, int nactu, int N, int device);
int
do_statmat(float *statcov,long dim, float *xpos, float *ypos, float norm, int device);
int
add_md(float *o_matrix, float *i_matrix, float *i_vector, int N,int device);
#endif // _sutra_controller_H_