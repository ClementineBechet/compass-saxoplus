#include <sutra_centroider_bpcog.h>
#include <string>

sutra_centroider_bpcog::sutra_centroider_bpcog(carma_context *context,
    long nwfs, long nvalid, float offset, float scale, int device, int nmax) {
  this->type = "bpcog";

  this->current_context = context;

  this->device = device;
  context->set_activeDevice(device);
  this->nwfs = nwfs;
  this->nvalid = nvalid;
  this->offset = offset;
  this->scale = scale;

  this->nmax = nmax;

}

sutra_centroider_bpcog::~sutra_centroider_bpcog() {
}

int sutra_centroider_bpcog::init_bincube(sutra_wfs *wfs) {
  return EXIT_SUCCESS;
}

int sutra_centroider_bpcog::set_nmax(int nmax) {
  this->nmax = nmax;

  return EXIT_SUCCESS;
}

int sutra_centroider_bpcog::get_cog(float *cube, float *subsum,
    float *centroids, int nvalid, int npix, int ntot) {
  // brightest pixels cog
  subap_centromax(npix * npix, nvalid, cube, centroids, npix, this->nmax, this->scale,
      this->offset);

  return EXIT_SUCCESS;
}

int sutra_centroider_bpcog::get_cog(sutra_wfs *wfs, carma_obj<float> *slopes) {
  return this->get_cog(wfs->d_bincube->getData(), wfs->d_subsum->getData(),
      slopes->getData(), wfs->nvalid, wfs->npix, wfs->d_bincube->getNbElem());
}

int sutra_centroider_bpcog::get_cog(sutra_wfs *wfs) {
  return this->get_cog(wfs, wfs->d_slopes);
}

int sutra_centroider_bpcog::get_cog_async(carma_streams *streams, float *cube,
    float *subsum, float *centroids, int nvalid, int npix) {
  // TODO: implemente sutra_centroider_bpcog::get_cog_async
  cerr << "get_cog_async not implemented\n";
  return EXIT_SUCCESS;
}

int sutra_centroider_bpcog::get_cog_async(sutra_wfs *wfs,
    carma_obj<float> *slopes) {
  return this->get_cog_async(wfs->streams, wfs->d_bincube->getData(),
      wfs->d_subsum->getData(), slopes->getData(), wfs->nvalid, wfs->npix);
}

int sutra_centroider_bpcog::get_cog_async(sutra_wfs *wfs) {
  return this->get_cog_async(wfs, wfs->d_slopes);
}