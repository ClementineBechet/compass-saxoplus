#include <sutra_controller.h>
#include <string>

sutra_controller::sutra_controller(carma_context* context, int nslope,
    int nactu) {
  this->current_context = context;
  this->device = context->get_activeDevice();

  long dims_data1[2] = { 1, 0 };

  dims_data1[1] = nslope;
  this->d_centroids = new carma_obj<float>(context, dims_data1);
  dims_data1[1] = nactu;
  this->d_com = new carma_obj<float>(context, dims_data1);

}

sutra_controller::~sutra_controller() {
  delete this->d_centroids;
  delete this->d_com;
}
