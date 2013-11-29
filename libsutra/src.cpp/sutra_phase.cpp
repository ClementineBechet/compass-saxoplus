#include <sutra_phase.h>

sutra_phase::sutra_phase(carma_context *current_context, long size)
{
  this->screen_size = size;
  
  long *dims_data2 = new long[3];
  dims_data2[0] = 2; 
  dims_data2[1] = this->screen_size; 
  dims_data2[2] = this->screen_size; 
  
  this->d_screen = new carma_obj<float>(current_context, dims_data2);
  delete[] dims_data2;
}

sutra_phase::~sutra_phase()
{
  delete this->d_screen;
}