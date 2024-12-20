// This file is part of COMPASS <https://github.com/COSMIC-RTC/compass>
//
// COMPASS is free software: you can redistribute it and/or modify it under the terms of the GNU
// Lesser General Public License as published by the Free Software Foundation, either version 3 of
// the License, or any later version.
//
// COMPASS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with COMPASS. If
// not, see <https://www.gnu.org/licenses/>
//
//  Copyright (C) 2011-2024 COSMIC Team <https://github.com/COSMIC-RTC/compass>

//! \file      sutra_perfectCoronograph.cpp
//! \ingroup   libsutra
//! \class     SutraPerfectCoronograph
//! \brief     this class provides the coronograph features to COMPASS
//! \author    COSMIC Team <https://github.com/COSMIC-RTC/compass>
//! \date      2022/01/24

#include <sutra_perfectCoronagraph.hpp>

SutraPerfectCoronagraph::SutraPerfectCoronagraph(CarmaContext *context, SutraSource *d_source,
                                    int32_t im_dimx, int32_t im_dimy, float *wavelength, int32_t nWavelength,
                                    int32_t device):
    SutraCoronagraph(context, "perfect", d_source, im_dimx, im_dimy, wavelength, nWavelength, device),
    tmp_mft(nullptr) {

        AA = {
            {"img", std::vector<CarmaObj<cuFloatComplex>*>(nWavelength, nullptr)},
            {"psf", std::vector<CarmaObj<cuFloatComplex>*>(nWavelength, nullptr)}
        };
        BB = {
            {"img", std::vector<CarmaObj<cuFloatComplex>*>(nWavelength, nullptr)},
            {"psf", std::vector<CarmaObj<cuFloatComplex>*>(nWavelength, nullptr)}
        };
        norm = {
            {"img", std::vector<float>(nWavelength, 1)},
            {"psf", std::vector<float>(nWavelength, 1)}
        };
    }

int32_t SutraPerfectCoronagraph::set_mft(cuFloatComplex *A, cuFloatComplex *B, float* norm0,
                                        std::string mftType) {
    if(AA.count(mftType) < 1) {
        std::cout << "Invalid mftType. Must be img or psf" << std::endl;
        return EXIT_FAILURE;
    }

    int64_t dims[3];
    dims[0] = 2;
    for (int32_t i = 0; i < wavelength.size() ; i++) {
        dims[1] = imageDimx;
        dims[2] = pupDimx;
        AA[mftType][i] = new CarmaObj<cuFloatComplex>(current_context, dims, A + i * imageDimx * pupDimx);
        dims[1] = pupDimy;
        dims[2] = imageDimy;
        BB[mftType][i] = new CarmaObj<cuFloatComplex>(current_context, dims, B + i * imageDimy * pupDimy);
        norm[mftType][i] = norm0[i];
    }

    if (tmp_mft == nullptr) {
        dims[1] = imageDimx;
        dims[2] = pupDimy;
        tmp_mft = new CarmaObj<cuFloatComplex>(current_context, dims);
    }
    return EXIT_SUCCESS;
}

int32_t SutraPerfectCoronagraph::_compute_image(bool psf, bool accumulate, bool remove_coro) {
    CarmaObj<float> *img_se = d_image_se;
    CarmaObj<float> *img_le = d_image_le;
    std::vector<CarmaObj<cuFloatComplex>*> mftA = AA["img"];
    std::vector<CarmaObj<cuFloatComplex>*> mftB = BB["img"];
    std::vector<float> mftNorm = norm["img"];
    if (psf) {
        img_se = d_psf_se;
        img_le = d_psf_le;
        mftA = AA["psf"];
        mftB = BB["psf"];
        mftNorm = norm["psf"];
    }

    img_se->reset();
    for (int32_t i = 0 ; i < wavelength.size(); i++) {
        compute_electric_field(i);
        if(!remove_coro) {
            cuFloatComplex sum = d_electric_field->sum();
            remove_complex_avg(d_electric_field->get_data(), d_electric_field->sum(),
                            d_pupil->get_data(), d_source->d_wherephase->get_nb_elements(),
                            pupDimx, pupDimy, current_context->get_device(device));
        }
        mft(mftA[i], mftB[i], tmp_mft, d_electric_field, d_complex_image, mftNorm[i]);
        accumulate_abs2(d_complex_image->get_data(), img_se->get_data(),
                        img_se->get_nb_elements(), current_context->get_device(device));
    }

    if(accumulate) {
        img_le->axpy(1.0f, img_se, 1, 1);
        if(psf)
            cntPsf += 1;
        else
            cntImg += 1;
    }
    return EXIT_SUCCESS;
}

int32_t SutraPerfectCoronagraph::compute_psf(bool accumulate) {
    return _compute_image(true, accumulate, true);
}

int32_t SutraPerfectCoronagraph::compute_image(bool accumulate) {
    return _compute_image(false, accumulate, false);
}