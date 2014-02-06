#include <sutra_centroider_wcog.h>
#include <string>

sutra_centroider_wcog::sutra_centroider_wcog(carma_context *context, long nwfs,
    long nvalid, float offset, float scale, int device) {
  this->type = "wcog";

  this->current_context = context;

  this->device = device;
  context->set_activeDevice(device);
  this->nwfs = nwfs;
  this->nvalid = nvalid;
  this->offset = offset;
  this->scale = scale;

  this->npix=0;
  this->d_weights = 0L;
}

sutra_centroider_wcog::~sutra_centroider_wcog() {
}

int sutra_centroider_wcog::init_bincube(sutra_wfs *wfs) {
  return EXIT_SUCCESS;
}

int sutra_centroider_wcog::init_weights(sutra_wfs *wfs) {
  if (this->d_weights != 0L)
    delete this->d_weights;

  this->npix = wfs->npix;

  long *dims_data3 = new long[4];
  dims_data3[0] = 3;
  dims_data3[1] = this->npix;
  dims_data3[2] = this->npix;
  dims_data3[3] = this->nvalid;

  current_context->set_activeDevice(device);
  this->d_weights = new carma_obj<float>(current_context, dims_data3);

  delete[] dims_data3;

  return EXIT_SUCCESS;
}

int sutra_centroider_wcog::load_weights(float *weights, int ndim) {
  if (ndim == 3)
    this->d_weights->host2device(weights);
  else {
    // weights is a 2d array
    // same weight for each subap
    float *tmp; ///< Input data
    cutilSafeCall(
        cudaMalloc((void** )&tmp, sizeof(float) * this->npix * this->npix));
    cutilSafeCall(
        cudaMemcpy(tmp, weights, sizeof(float) * this->npix * this->npix,
            cudaMemcpyHostToDevice));
    fillweights(this->d_weights->getData(), tmp, this->npix,
        this->d_weights->getNbElem(), this->device);
    cutilSafeCall(cudaFree(tmp));
  }

  return EXIT_SUCCESS;
}

int sutra_centroider_wcog::get_cog(float *cube, float *subsum, float *centroids,
    int nvalid, int npix, int ntot) {
  // wcog
  subap_reduce(ntot, npix * npix, nvalid, cube, subsum,
      this->d_weights->getData());

  get_centroids(ntot, npix * npix, nvalid, npix, cube, centroids, subsum,
      this->d_weights->getData(), this->scale, this->offset);

  return EXIT_SUCCESS;
}

int sutra_centroider_wcog::get_cog(sutra_wfs *wfs, carma_obj<float> *slopes) {
  return this->get_cog(wfs->d_bincube->getData(), wfs->d_subsum->getData(),
      slopes->getData(), wfs->nvalid, wfs->npix, wfs->d_bincube->getNbElem());
}

int sutra_centroider_wcog::get_cog(sutra_wfs *wfs) {
  return this->get_cog(wfs, wfs->d_slopes);
}

int sutra_centroider_wcog::get_cog_async(carma_streams *streams, float *cube,
    float *subsum, float *centroids, int nvalid, int npix) {
  //TODO: Implement sutra_centroider_wcog::get_cog_async
  cerr << "get_cog_async not implemented\n";
  return EXIT_SUCCESS;
}

int sutra_centroider_wcog::get_cog_async(sutra_wfs *wfs,
    carma_obj<float> *slopes) {
  return this->get_cog_async(wfs->streams, wfs->d_bincube->getData(),
      wfs->d_subsum->getData(), slopes->getData(), wfs->nvalid, wfs->npix);
}

int sutra_centroider_wcog::get_cog_async(sutra_wfs *wfs) {
  return this->get_cog_async(wfs, wfs->d_slopes);
}