// -----------------------------------------------------------------------------
//  This file is part of COMPASS <https://anr-compass.github.io/compass/>
//
//  Copyright (C) 2011-2022 COMPASS Team <https://github.com/ANR-COMPASS>
//  All rights reserved.

// -----------------------------------------------------------------------------

//! \file      sutra_coronagraph.h
//! \ingroup   libsutra
//! \class     SutraCoronagraph
//! \brief     this class provides the coronagraph features to COMPASS
//! \author    COMPASS Team <https://github.com/ANR-COMPASS>
//! \version   5.3.0
//! \date      2022/01/24

#ifndef _SUTRA_CORONAGRAPH_H_
#define _SUTRA_CORONAGRAPH_H_

#include <carma_cublas.h>
#include <sutra_utils.h>
#include <sutra_source.h>
#include <tuple>
#include <vector>

class SutraCoronagraph {
    public:
        int device;
        std::string type;
        long cnt;
        int imageDimx;
        int imageDimy;
        int pupDimx;
        int pupDimy;
        std::vector<float> wavelength;
        CarmaContext *current_context;
        CarmaObj<float> *d_image_se;
        CarmaObj<float> *d_image_le;
        CarmaObj<float> *d_amplitude;

        CarmaObj<cuFloatComplex> *d_electric_field;
        CarmaObj<cuFloatComplex> *d_complex_image;
        CarmaObj<float> *d_pupil;

        SutraSource* d_source;

    public:
        virtual ~SutraCoronagraph()=default;
        virtual int compute_image(bool accumulate) = 0;
        int reset();
        int compute_electric_field(int wavelengthIndex);
    protected:
        virtual int propagate() = 0;
        SutraCoronagraph(CarmaContext *context, std::string type, SutraSource *d_source, 
                            int dimx, int dimy,float *wavelength, int nWavelength, int device);
        int mft(CarmaObj<cuFloatComplex> *A, CarmaObj<cuFloatComplex> *B, 
                CarmaObj<cuFloatComplex> *Ainput,
                CarmaObj<cuFloatComplex> *input, CarmaObj<cuFloatComplex> *output, float norm);

}

int compute_electric_field(cuFloatComplex *electric_field, float* phase_opd, float scale, 
                            float* amplitude, float* mask, int dimx, int dimy, CarmaDevice *device);
int remove_complex_avg(cuFloatComplex *electric_field, cuFloatComplex sum, float* mask, int Nvalid, 
                        int dimx, int dimy, CarmaDevice *device);
int accumulate_abs2(cuFloatComplex *img, float* abs2img, int N, CarmaDevice *device);
#endif //_SUTRA_CORONAGRAPH_H_
