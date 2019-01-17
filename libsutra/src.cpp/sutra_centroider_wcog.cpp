#include <sutra_centroider_wcog.h>
#include <string>

sutra_centroider_wcog::sutra_centroider_wcog(carma_context *context,
                                             sutra_wfs *wfs, long nvalid,
                                             float offset, float scale,
                                             int device)
    : sutra_centroider(context, wfs, nvalid, offset, scale, device) {
  context->set_activeDevice(device, 1);

  this->nslopes = 2 * nvalid;
  if (wfs != nullptr)
    this->npix = wfs->npix;
  else
    this->npix = 0;
  this->d_weights = 0L;
  long dims_data2[2] = {1, nslopes};
  this->d_centroids_ref =
      new carma_obj<float>(this->current_context, dims_data2);
  this->d_centroids_ref->reset();
}

sutra_centroider_wcog::~sutra_centroider_wcog() {}

string sutra_centroider_wcog::get_type() { return "wcog"; }

int sutra_centroider_wcog::init_weights() {
  current_context->set_activeDevice(device, 1);
  if (this->d_weights != 0L) delete this->d_weights;

  long *dims_data3 = new long[4];
  dims_data3[0] = 3;
  dims_data3[1] = this->npix;
  dims_data3[2] = this->npix;
  dims_data3[3] = this->nvalid;

  current_context->set_activeDevice(device, 1);
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
    float *tmp;  ///< Input data
    carmaSafeCall(
        cudaMalloc((void **)&tmp, sizeof(float) * this->npix * this->npix));
    carmaSafeCall(cudaMemcpy(tmp, weights,
                             sizeof(float) * this->npix * this->npix,
                             cudaMemcpyHostToDevice));
    fillweights<float>(*(this->d_weights), tmp, this->npix,
                       this->d_weights->getNbElem(),
                       this->current_context->get_device(device));
    carmaSafeCall(cudaFree(tmp));
  }

  return EXIT_SUCCESS;
}

int sutra_centroider_wcog::set_npix(int npix) {
  this->npix = npix;
  return EXIT_SUCCESS;
}
int sutra_centroider_wcog::get_cog(float *img, float *intensities,
                                   float *centroids, int nvalid, int npix,
                                   int ntot) {
  // wcog
  // TODO: Implement sutra_centroider_wcog::get_cog_async
  // subap_reduce<float>(ntot, npix * npix, nvalid, cube, intensities,
  //                     *(this->d_weights),
  //                     this->current_context->get_device(device));

  // get_centroids<float>(ntot, npix * npix, nvalid, npix, cube, centroids,
  //                      intensities, *(this->d_weights), this->scale,
  //                      this->offset,
  //                      this->current_context->get_device(device));

  get_centroids(ntot, (npix * npix), nvalid, npix, img, centroids,
                this->d_centroids_ref->getData(), this->d_validx->getData(),
                this->d_validy->getData(), intensities,
                this->d_weights->getData(), this->scale, this->offset,
                current_context->get_device(device));

  return EXIT_SUCCESS;
}

int sutra_centroider_wcog::get_cog(float *intensities, float *slopes,
                                   bool noise) {
  if (this->wfs != nullptr) {
    if (noise || wfs->roket == false)
      return this->get_cog(*(wfs->d_binimg), intensities, slopes, wfs->nvalid,
                           wfs->npix, wfs->d_binimg->getDims()[1]);
    else
      return this->get_cog(*(wfs->d_binimg_notnoisy), intensities, slopes,
                           wfs->nvalid, wfs->npix, wfs->d_binimg->getDims()[1]);
  }
  DEBUG_TRACE("this->wfs was not initialized");
  return EXIT_FAILURE;
}

int sutra_centroider_wcog::get_cog() {
  if (this->wfs != nullptr)
    return this->get_cog(*(wfs->d_intensities), *(wfs->d_slopes), true);
  DEBUG_TRACE("this->wfs was not initialized");
  return EXIT_FAILURE;
}
