#include <sutra_centroider.h>

template <class Tin, class Tout>
sutra_centroider<Tin, Tout>::sutra_centroider(carma_context *context,
                                              sutra_wfs *wfs, long nvalid,
                                              float offset, float scale,
                                              int device) {
  this->current_context = context;
  this->device = device;
  context->set_activeDevice(device, 1);
  this->wfs = wfs;

  this->nvalid = nvalid;
  this->offset = offset;
  this->scale = scale;
  this->nslopes = 0;
  this->npix = 0;
  this->nxsub = 0;

  long dims_data[2] = {1, this->nvalid};
  this->d_intensities = new carma_obj<float>(current_context, dims_data);
  this->d_intensities->reset();

  this->d_centroids_ref = nullptr;
  this->d_img = nullptr;
  this->d_img_raw = nullptr;
  this->d_validx = nullptr;
  this->d_validy = nullptr;
  this->d_dark = nullptr;
  this->d_flat = nullptr;
  this->d_bincube = nullptr;
  this->d_validMask = nullptr;
}

template <class Tin, class Tout>
sutra_centroider<Tin, Tout>::~sutra_centroider() {
  if (this->d_intensities != nullptr) delete this->d_intensities;
  if (this->d_centroids_ref != nullptr) delete this->d_centroids_ref;
  if (this->d_img != nullptr) delete this->d_img;
  if (this->d_img_raw != nullptr) delete this->d_img_raw;
  if (this->d_validx != nullptr) delete this->d_validx;
  if (this->d_validy != nullptr) delete this->d_validy;
  if (this->d_dark != nullptr) delete this->d_dark;
  if (this->d_flat != nullptr) delete this->d_flat;
  if (this->d_bincube != nullptr) delete this->d_bincube;
  if (this->d_validMask != nullptr) delete this->d_validMask;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::set_scale(float scale) {
  this->scale = scale;
  return EXIT_SUCCESS;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::set_nxsub(int nxsub) {
  this->nxsub = nxsub;
  return EXIT_SUCCESS;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::set_dark(float *dark, int n) {
  current_context->set_activeDevice(device, 1);
  if (this->d_dark == nullptr) {
    long dims_data2[3] = {2, n, n};
    this->d_dark = new carma_obj<float>(current_context, dims_data2);
  }
  this->d_dark->host2device(dark);
  return EXIT_SUCCESS;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::set_flat(float *flat, int n) {
  current_context->set_activeDevice(device, 1);
  if (this->d_flat == nullptr) {
    long dims_data2[3] = {2, n, n};
    this->d_flat = new carma_obj<float>(current_context, dims_data2);
  }
  this->d_flat->host2device(flat);
  return EXIT_SUCCESS;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::get_validMask() {
  this->current_context->set_activeDevice(this->device, 1);
  if (this->d_validMask == nullptr) {
    if (this->d_img == nullptr) {
      std::cout << "RTC image has not been initialized" << std::endl;
      return EXIT_FAILURE;
    }
    this->d_validMask = new carma_obj<int>(current_context, d_img->getDims());
    this->d_validMask->reset();
  }

  fill_validMask(this->d_validMask->getDims(1), this->npix, this->nvalid,
                 this->d_validMask->getData(), this->d_validx->getData(),
                 this->d_validy->getData(),
                 current_context->get_device(device));

  return EXIT_SUCCESS;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::calibrate_img() {
  current_context->set_activeDevice(device, 1);

  if (this->d_img_raw == nullptr) {
    std::cout << "Image not initialized\n" << std::endl;
    return EXIT_FAILURE;
  }
  if (this->d_dark == nullptr) {
    this->d_dark =
        new carma_obj<float>(current_context, this->d_img->getDims());
    this->d_dark->reset();
  }
  if (this->d_flat == nullptr) {
    this->d_flat =
        new carma_obj<float>(current_context, this->d_img->getDims());
    this->d_flat->memSet(1.f);
  }

  calibration<Tin>(this->d_img_raw->getData(), this->d_img->getData(),
                   this->d_dark->getData(), this->d_flat->getData(),
                   this->d_img->getNbElem(),
                   this->current_context->get_device(this->device));

  return EXIT_SUCCESS;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::load_img(Tin *img, int n) {
  current_context->set_activeDevice(device, 1);
  if (this->d_img_raw == nullptr) {
    long dims_data2[3] = {2, n, n};
    this->d_img_raw = new carma_obj<Tin>(current_context, dims_data2);
    this->d_img = new carma_obj<float>(current_context, dims_data2);
  }
  this->d_img_raw->host2device(img);
  return EXIT_SUCCESS;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::load_img_gpu(carma_obj<Tin> *d_img_raw) {
  current_context->set_activeDevice(device, 1);
  if (this->d_img_raw == nullptr) {
    this->d_img_raw = new carma_obj<Tin>(current_context, d_img_raw->getDims());
    this->d_img = new carma_obj<float>(current_context, d_img_raw->getDims());
  }
  this->d_img_raw->copyFrom(d_img_raw->getData(), this->d_img_raw->getNbElem());
  return EXIT_SUCCESS;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::set_npix(int npix) {
  this->npix = npix;

  return EXIT_SUCCESS;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::load_validpos(int *ivalid, int *jvalid,
                                               int N) {
  current_context->set_activeDevice(device, 1);
  if (this->d_validx == nullptr) {
    long dims_data[2] = {1, N};
    this->d_validx = new carma_obj<int>(current_context, dims_data);
    this->d_validy = new carma_obj<int>(current_context, dims_data);
  }

  this->d_validx->host2device(ivalid);
  this->d_validy->host2device(jvalid);

  return EXIT_SUCCESS;
}

template <class Tin, class Tout>
int sutra_centroider<Tin, Tout>::set_centroids_ref(float *centroids_ref) {
  this->d_centroids_ref->host2device(centroids_ref);
  return EXIT_SUCCESS;
}

template class sutra_centroider<float, float>;
template class sutra_centroider<uint16_t, float>;
#ifdef CAN_DO_HALF
template class sutra_centroider<float, half>;
template class sutra_centroider<uint16_t, half>;
#endif
