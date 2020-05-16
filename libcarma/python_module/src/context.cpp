// -----------------------------------------------------------------------------
//  This file is part of COMPASS <https://anr-compass.github.io/compass/>
//
//  Copyright (C) 2011-2019 COMPASS Team <https://github.com/ANR-COMPASS>
//  All rights reserved.
//  Distributed under GNU - LGPL
//
//  COMPASS is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser
//  General Public License as published by the Free Software Foundation, either version 3 of the License,
//  or any later version.
//
//  COMPASS: End-to-end AO simulation tool using GPU acceleration
//  The COMPASS platform was designed to meet the need of high-performance for the simulation of AO systems.
//
//  The final product includes a software package for simulating all the critical subcomponents of AO,
//  particularly in the context of the ELT and a real-time core based on several control approaches,
//  with performances consistent with its integration into an instrument. Taking advantage of the specific
//  hardware architecture of the GPU, the COMPASS tool allows to achieve adequate execution speeds to
//  conduct large simulation campaigns called to the ELT.
//
//  The COMPASS platform can be used to carry a wide variety of simulations to both testspecific components
//  of AO of the E-ELT (such as wavefront analysis device with a pyramid or elongated Laser star), and
//  various systems configurations such as multi-conjugate AO.
//
//  COMPASS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
//  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License along with COMPASS.
//  If not, see <https://www.gnu.org/licenses/lgpl-3.0.txt>.
// -----------------------------------------------------------------------------

//! \file      context.cpp
//! \ingroup   libcarma
//! \brief     this file provides pybind wrapper for CarmaContext
//! \author    COMPASS Team <https://github.com/ANR-COMPASS>
//! \version   5.0.0
//! \date      2011/01/28
//! \copyright GNU Lesser General Public License

#include "declare_name.hpp"

#include <carma.h>

#include <wyrm>
namespace py = pybind11;

void declare_carmaWrap_context(py::module &mod) {
  py::class_<CarmaDevice>(mod, "device")
      .def_property_readonly("id", &CarmaDevice::get_properties)
      .def_property_readonly("compute_perf", &CarmaDevice::get_compute_perf)
      .def_property_readonly("cores_per_sm", &CarmaDevice::get_cores_per_sm)
      .def_property_readonly("p2p_activate", &CarmaDevice::is_p2p_activate)
      .def_property_readonly("name", &CarmaDevice::get_name)
      .def_property_readonly("total_mem", &CarmaDevice::get_total_mem)
      .def_property_readonly("total_mem", &CarmaDevice::get_free_mem);

  py::class_<CarmaContext>(mod, "context")
      .def_property_readonly("ndevices", &CarmaContext::get_ndevice)

      .def_static("get_instance", &CarmaContext::instance,
                  py::return_value_policy::reference)
      .def_static("get_instance_1gpu", &CarmaContext::instance_1gpu,
                  py::return_value_policy::reference)
      .def_static("get_instance_ngpu",
                  wy::colCast(CarmaContext::instance_ngpu),
                  py::return_value_policy::reference)

      // .def(py::init([](py::buffer data, ::CORBA::Boolean copy) {
      //   py::buffer_info info = data.request();
      //   if(info.format != py::format_descriptor<::CORBA::ULong>::format())
      //     throw invalid_argument("Buffer given has an incorrect format,
      //     expected: uint32");
      //   ::CORBA::ULong *ptr;
      //   if (copy == 0) {
      //     ptr = reinterpret_cast<::CORBA::ULong *>(info.ptr);
      //   } else {
      //     ssize_t size = info.itemsize * info.size;
      //     ptr = new ::CORBA::ULong[info.size];
      //     memcpy(ptr, info.ptr, size);
      //   }
      //   return unique_ptr<B::Dims>(new B::Dims(info.size, info.size, ptr,
      //   copy));
      // }))

      .def_property_readonly("ndevice", &CarmaContext::get_ndevice)
      .def_property_readonly("active_device", &CarmaContext::get_active_device)
      .def_property_readonly("activeRealDevice",
                             &CarmaContext::get_active_real_device)
      .def_property_readonly("cudaRuntimeGetVersion",
                             &CarmaContext::get_cuda_runtime_get_version)
      .def_property_readonly("driverVersion",
                             &CarmaContext::get_cuda_driver_get_version)
      .def("get_device", &CarmaContext::get_device,
           py::return_value_policy::reference)
      .def("set_active_device",
           [](CarmaContext &cc, int new_device) {
             return cc._set_active_device(new_device, 1, __FILE__, __LINE__);
           })
      .def("set_active_device_force",
           [](CarmaContext &cc, int new_device) {
             return cc._set_active_device_force(new_device, 0, __FILE__, __LINE__);
           })
      // .def("set_active_deviceForCpy", &CarmaContext::set_active_deviceForCpy);
      .def(
          "activate_tensor_cores",
          [](CarmaContext &cc, bool flag) {
            int ndevices = cc.get_ndevice();
            for (int i = 0; i < ndevices; i++) {
              cc.get_device(i)->set_cublas_math_mode(flag);
            }
          },
          "Set the cublas math mode using tensor cores or not",
          py::arg("flag"));

  mod.def("deviceSync", &__carma_safe_device_synchronize);
}
