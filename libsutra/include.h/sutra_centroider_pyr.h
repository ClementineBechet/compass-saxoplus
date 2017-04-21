#ifndef _SUTRA_CENTROIDER_PYR_H_
#define _SUTRA_CENTROIDER_PYR_H_

#include <sutra_centroider.h>

enum type_CoG {local=0, global};

class sutra_centroider_pyr: public sutra_centroider {
private:
  string pyr_type;

public:
  sutra_centroider_pyr(carma_context *context, sutra_sensors *sensors, int nwfs, long nvalid,
      float offset, float scale, int device);
  sutra_centroider_pyr(const sutra_centroider_pyr& centroider);
  ~sutra_centroider_pyr();

  string get_type();
  int set_valid_thresh(float valid_thresh);
  float get_valid_thresh();

  int set_cog_type(int type);

  int get_pyr(float *cube, float *subsum, float *centroids, int *subindx,
      int *subindy, int nvalid, int ns, int nim);
  int get_cog(carma_streams *streams, float *cube, float *subsum, float *centroids,
      int nvalid, int npix, int ntot);
  int get_cog(float *subsum, float *slopes, bool noise);
  int get_cog();

private:
  float valid_thresh;
  type_CoG method;
};

#endif // _SUTRA_CENTROIDER_PYR_H_
