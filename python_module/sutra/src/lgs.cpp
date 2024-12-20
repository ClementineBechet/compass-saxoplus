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

//! \file      lgs.cpp
//! \ingroup   libsutra
//! \brief     this file provides pybind wrapper for SutraLGS
//! \author    COSMIC Team <https://github.com/COSMIC-RTC/compass>
//! \date      2022/01/24

#include "sutraUtils.hpp"

#include <sutra_lgs.hpp>

namespace py = pybind11;

int32_t lgs_init(SutraLGS &sl, int32_t nprof, float hg, float h0, float deltah,
             float pixsize, ArrayFStyle<float> &doffaxis, 
             ArrayFStyle<float> &prof1d, ArrayFStyle<float> &profcum, 
             ArrayFStyle<float> &beam, 
             ArrayFStyle<std::complex<float>> &ftbeam,
             ArrayFStyle<float> &azimuth) {
    return sl.lgs_init(nprof, hg, h0, deltah, pixsize, doffaxis.mutable_data(), 
                       prof1d.mutable_data(), profcum.mutable_data(), beam.mutable_data(), 
                       reinterpret_cast<cuFloatComplex *>(ftbeam.mutable_data()), azimuth.mutable_data());
}

void declare_lgs(py::module &mod) {
  py::class_<SutraLGS>(mod, "LGS")

      //  ██████╗ ██████╗  ██████╗ ██████╗ ███████╗██████╗ ████████╗██╗   ██╗
      //  ██╔══██╗██╔══██╗██╔═══██╗██╔══██╗██╔════╝██╔══██╗╚══██╔══╝╚██╗ ██╔╝
      //  ██████╔╝██████╔╝██║   ██║██████╔╝█████╗  ██████╔╝   ██║    ╚████╔╝
      //  ██╔═══╝ ██╔══██╗██║   ██║██╔═══╝ ██╔══╝  ██╔══██╗   ██║     ╚██╔╝
      //  ██║     ██║  ██║╚██████╔╝██║     ███████╗██║  ██║   ██║      ██║
      //  ╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝      ╚═╝

      .def_property_readonly(
          "device", [](SutraLGS &sl) { return sl.device; }, "GPU device index")

      .def_property_readonly(
          "nvalid", [](SutraLGS &sl) { return sl.nvalid; }, "TODO: docstring")

      .def_property_readonly(
          "npix", [](SutraLGS &sl) { return sl.npix; }, "TODO: docstring")

      .def_property_readonly(
          "nmaxhr", [](SutraLGS &sl) { return sl.nmaxhr; },
          "Size of HR support")

      .def_property_readonly(
          "hg", [](SutraLGS &sl) { return sl.hg; }, "TODO: docstring")

      .def_property_readonly(
          "h0", [](SutraLGS &sl) { return sl.h0; }, "TODO: docstring")

      .def_property_readonly(
          "deltah", [](SutraLGS &sl) { return sl.deltah; }, "TODO: docstring")

      .def_property_readonly(
          "pixsize", [](SutraLGS &sl) { return sl.pixsize; },
          "Pixel size on sky[arcsec]")

      .def_property_readonly(
          "nprof", [](SutraLGS &sl) { return sl.nprof; }, "TODO: docstring")

      .def_property_readonly(
          "d_doffaxis", [](SutraLGS &sl) { return sl.d_doffaxis; },
          "TODO: docstring")

      .def_property_readonly(
          "d_azimuth", [](SutraLGS &sl) { return sl.d_azimuth; },
          "TODO: docstring")

      .def_property_readonly(
          "d_prof1d", [](SutraLGS &sl) { return sl.d_prof1d; },
          "TODO: docstring")

      .def_property_readonly(
          "d_profcum", [](SutraLGS &sl) { return sl.d_profcum; },
          "TODO: docstring")

      .def_property_readonly(
          "d_prof2d", [](SutraLGS &sl) { return sl.d_prof2d; },
          "TODO: docstring")

      .def_property_readonly(
          "d_beam", [](SutraLGS &sl) { return sl.d_beam; }, "TODO: docstring")

      .def_property_readonly(
          "d_ftbeam", [](SutraLGS &sl) { return sl.d_ftbeam; },
          "TODO: docstring")

      .def_property_readonly(
          "d_lgskern", [](SutraLGS &sl) { return sl.d_lgskern; },
          "TODO: docstring")

      .def_property_readonly(
          "d_ftlgskern", [](SutraLGS &sl) { return sl.d_ftlgskern; },
          "TODO: docstring")

      //  ███╗   ███╗███████╗████████╗██╗  ██╗ ██████╗ ██████╗ ███████╗
      //  ████╗ ████║██╔════╝╚══██╔══╝██║  ██║██╔═══██╗██╔══██╗██╔════╝
      //  ██╔████╔██║█████╗     ██║   ███████║██║   ██║██║  ██║███████╗
      //  ██║╚██╔╝██║██╔══╝     ██║   ██╔══██║██║   ██║██║  ██║╚════██║
      //  ██║ ╚═╝ ██║███████╗   ██║   ██║  ██║╚██████╔╝██████╔╝███████║
      //  ╚═╝     ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚═════╝ ╚══════╝

      .def("lgs_init", &lgs_init, R"pbdoc(
    Initialize LGS object

    Args:
        nprof: (int): TODO: docstring

        hg: (float):

        h0: (float):

        deltah: (float):

        pixsize: (float):

        doffaxis:(np.array[ndim= , dtype=np.float32]):

        prof1d:(np.array[ndim= , dtype=np.float32]):

        profcum:(np.array[ndim= , dtype=np.float32]):

        beam:(np.array[ndim= , dtype=np.float32]):

        ftbeam:(np.array[ndim= , dtype=np.complex64]):

        azimuth:(np.array[ndim= , dtype=np.float32]):
    )pbdoc",
           py::arg("nprof"), py::arg("hg"), py::arg("h0"), py::arg("deltah"),
           py::arg("pixsize"), py::arg("doffaxis"), py::arg("prof1d"),
           py::arg("profcum"), py::arg("beam"), py::arg("ftbeam"),
           py::arg("azimuth"))

      //  ███████╗███████╗████████╗████████╗███████╗██████╗ ███████╗
      //  ██╔════╝██╔════╝╚══██╔══╝╚══██╔══╝██╔════╝██╔══██╗██╔════╝
      //  ███████╗█████╗     ██║      ██║   █████╗  ██████╔╝███████╗
      //  ╚════██║██╔══╝     ██║      ██║   ██╔══╝  ██╔══██╗╚════██║
      //  ███████║███████╗   ██║      ██║   ███████╗██║  ██║███████║
      //  ╚══════╝╚══════╝   ╚═╝      ╚═╝   ╚══════╝╚═╝  ╚═╝╚══════╝
      ;
};
