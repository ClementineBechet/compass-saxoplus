#include <sutra_centroider_tcog.h>
#include <string>

sutra_centroider_tcog::sutra_centroider_tcog(carma_context *context,
                                             sutra_wfs *wfs, long nvalid,
                                             float offset, float scale,
                                             int device)
    : sutra_centroider(context, wfs, nvalid, offset, scale, device) {
  context->set_activeDevice(device, 1);

  this->nslopes = 2 * nvalid;
  this->threshold = 0;
  long dims_data2[2] = {1, nslopes};
  this->d_centroids_ref =
      new carma_obj<float>(this->current_context, dims_data2);
  this->d_centroids_ref->reset();
}

sutra_centroider_tcog::~sutra_centroider_tcog() {}

string sutra_centroider_tcog::get_type() { return "tcog"; }

int sutra_centroider_tcog::set_threshold(float threshold) {
  this->threshold = threshold;

  return EXIT_SUCCESS;
}

int sutra_centroider_tcog::get_cog(float *img, float *intensities,
                                   float *centroids, int nvalid, int npix,
                                   int ntot) {
  current_context->set_activeDevice(device, 1);

  get_centroids(ntot, (npix * npix), nvalid, npix, img, centroids,
                this->d_centroids_ref->getData(), this->d_validx->getData(),
                this->d_validy->getData(), intensities, this->threshold,
                this->scale, this->offset, current_context->get_device(device));

  // TODO: Implement sutra_centroider_tcog::get_cog_async
  // #ifndef USE_OLD
  //   // Modif Nono !!!
  //   // Now subap_reduce modify cube to apply the threshold on the cube so
  //   // get_centroids should apply threshold a second time
  //   subap_reduce_new(ntot, npix * npix, nvalid, cube, intensities,
  //                    this->threshold,
  //                    this->current_context->get_device(device));

  //   get_centroids(ntot, npix * npix, nvalid, npix, cube, centroids,
  //   intensities,
  //                 this->threshold, this->scale, this->offset,
  //                 this->current_context->get_device(device));
  // #else
  //   subap_reduce(ntot, npix * npix, nvalid, cube, intensities,
  //   this->threshold,
  //                this->current_context->get_device(device));

  //   get_centroids(ntot, npix * npix, nvalid, npix, cube, centroids,
  //   intensities,
  //                 this->threshold, this->scale, this->offset,
  //                 this->current_context->get_device(device));
  // #endif
  return EXIT_SUCCESS;
}

int sutra_centroider_tcog::get_cog(float *intensities, float *slopes,
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
int sutra_centroider_tcog::get_cog() {
  if (this->wfs != nullptr)
    return this->get_cog(*(wfs->d_intensities), *(wfs->d_slopes), true);
  DEBUG_TRACE("this->wfs was not initialized");
  return EXIT_FAILURE;
}
