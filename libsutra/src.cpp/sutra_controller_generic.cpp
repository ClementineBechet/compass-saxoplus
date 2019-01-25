#include <sutra_controller_generic.h>

template <typename T>
sutra_controller_generic<T>::sutra_controller_generic(carma_context *context,
                                                      long nvalid, long nslope,
                                                      long nactu, T delay,
                                                      sutra_dms *dms,
                                                      int *idx_dms, int ndm)
    : sutra_controller<T>(context, nvalid, nslope, nactu, delay, dms, idx_dms,
                          ndm) {
  this->command_law = "integrator";
  long dims_data1[2] = {1, nactu};
  this->d_gain = new carma_obj<T>(this->current_context, dims_data1);
  this->d_decayFactor = new carma_obj<T>(this->current_context, dims_data1);
  this->d_compbuff = new carma_obj<T>(this->current_context, dims_data1);
  long dims_data2[3] = {2, nactu, nslope};
  this->d_cmat = new carma_obj<T>(this->current_context, dims_data2);
  this->gain = 0.3f;

  dims_data2[2] = nactu;
  this->d_matE = new carma_obj<T>(this->current_context, dims_data2);
}

template <typename T>
sutra_controller_generic<T>::~sutra_controller_generic() {
  this->current_context->set_activeDevice(this->device, 1);
  delete this->d_gain;
  delete this->d_decayFactor;
  delete this->d_cmat;
  delete this->d_matE;
  delete this->d_compbuff;
}

template <typename T>
string sutra_controller_generic<T>::get_type() {
  this->current_context->set_activeDevice(this->device, 1);
  return "generic";
}

template <typename T>
string sutra_controller_generic<T>::get_commandlaw() {
  this->current_context->set_activeDevice(this->device, 1);
  return this->command_law;
}

template <typename T>
int sutra_controller_generic<T>::set_mgain(float *gain) {
  this->current_context->set_activeDevice(this->device, 1);
  this->d_gain->host2device(gain);
  return EXIT_SUCCESS;
}

template <typename T>
int sutra_controller_generic<T>::set_gain(T gain) {
  this->current_context->set_activeDevice(this->device, 1);
  this->gain = gain;
  return EXIT_SUCCESS;
}

template <typename T>
int sutra_controller_generic<T>::set_decayFactor(float *decayFactor) {
  this->current_context->set_activeDevice(this->device, 1);
  this->d_decayFactor->host2device(decayFactor);
  return EXIT_SUCCESS;
}

template <typename T>
int sutra_controller_generic<T>::set_matE(float *matE) {
  this->current_context->set_activeDevice(this->device, 1);
  this->d_matE->host2device(matE);
  return EXIT_SUCCESS;
}

template <typename T>
int sutra_controller_generic<T>::set_cmat(float *cmat) {
  this->current_context->set_activeDevice(this->device, 1);
  this->d_cmat->host2device(cmat);
  return EXIT_SUCCESS;
}

template <typename T>
int sutra_controller_generic<T>::set_commandlaw(string law) {
  this->current_context->set_activeDevice(this->device, 1);
  this->command_law = law;
  return EXIT_SUCCESS;
}

template <typename T>
int sutra_controller_generic<T>::comp_com() {
  this->current_context->set_activeDevice(this->device, 1);

  if (this->command_law == "integrator") {
    // cublasSetStream(this->cublas_handle(),
    //                 current_context->get_device(device)->get_stream());
    carma_gemv(this->cublas_handle(), 'n', this->nactu(), this->nslope(),
               (T)(-1 * this->gain), this->d_cmat->getData(), this->nactu(),
               this->d_centroids->getData(), 1, (T)1.0f, this->d_com->getData(),
               1);

  } else {
    // CMAT*s(k)
    carma_gemv(this->cublas_handle(), 'n', this->nactu(), this->nslope(),
               (T)1.0f, this->d_cmat->getData(), this->nactu(),
               this->d_centroids->getData(), 1, (T)0.0f,
               this->d_compbuff->getData(), 1);
    // g*CMAT*s(k)
    mult_vect(this->d_compbuff->getData(), this->d_gain->getData(), (T)(-1.0f),
              this->nactu(), this->current_context->get_device(this->device));

    carma_gemv(this->cublas_handle(), 'n', this->nactu(), this->nactu(),
               (T)1.0f, this->d_matE->getData(), this->nactu(),
               this->d_com1->getData(), 1, (T)0.0f, this->d_com->getData(), 1);
    // v(k) = alpha*E*v(k-1)
    mult_vect(this->d_com->getData(), this->d_decayFactor->getData(), (T)1.0f,
              this->nactu(), this->current_context->get_device(this->device));
    // v(k) = alpha*E*v(k-1) + g*CMAT*s(k)
    this->d_com->axpy((T)1.0f, this->d_compbuff, 1, 1);
  }
  return EXIT_SUCCESS;
}

template class sutra_controller_generic<float>;
#ifdef CAN_DO_HALF
template class sutra_controller_generic<half>;
#endif
