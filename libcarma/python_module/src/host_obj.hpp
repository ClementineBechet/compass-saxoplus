#ifndef _WRAP_HOST_OBJ_H_
#define _WRAP_HOST_OBJ_H_

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#include "declare_name.hpp"
#include <type_list.hpp>
#include <carma.h>

namespace py = pybind11;



struct CarmaHostObjInterfacer {
  template <typename T> static void call(py::module &mod) {
    auto name = appendName<T>("host_obj_");
    using Class = carma_host_obj<T>;

    py::class_<Class>(mod, name.data(), py::buffer_protocol())
        .def(py::init([](const py::array_t<T, py::array::f_style |
                                                  py::array::forcecast> &data,
                         MemAlloc mallocType) {
               int ndim = data.ndim() + 1;
               std::vector<long> data_dims(ndim);
               data_dims[0] = data.ndim();
               copy(data.shape(), data.shape() + data.ndim(),
                    begin(data_dims) + 1);
               return std::unique_ptr<Class>(
                   new Class(data_dims.data(), (const T *)data.data()));
             }),
             "TODO", // TODO do the documentation...
             py::arg("h_data").none(false), py::arg("mallocType")=MA_MALLOC)

        .def(py::init([](const Class &data) {
               return std::unique_ptr<Class>(new Class(&data));
             }),
             "TODO", // TODO do the documentation...
             py::arg("d_data").none(false))

        .def_buffer([](Class &frame) -> py::buffer_info {

          const long *dims = frame.getDims();
          std::vector<ssize_t> shape(dims[0]);
          std::vector<ssize_t> strides(dims[0]);
          ssize_t stride = sizeof(T);

          // C-style
          // for (ssize_t dim(dims[0] - 1); dim >= 0; --dim)
          // {
          //   shape[dim] = dims[dim + 1];
          //   strides[dim] = stride;
          //   stride *= shape[dim];
          // }

          // F-style
          for (ssize_t dim(0); dim < dims[0]; ++dim) {
            shape[dim] = dims[dim + 1];
            strides[dim] = stride;
            stride *= shape[dim];
          }

          return py::buffer_info(frame.getData(), sizeof(T),
                                 py::format_descriptor<T>::format(), dims[0],
                                 shape, strides);
        })

        // .def("__repr__", &std::string)

        // const long *getDims()
        .def_property_readonly("shape",
                               [](Class &frame) -> py::array_t<long> {
                                 long nb_dim = frame.getDims(0);
                                 const long *c_dim = frame.getDims() + 1;
                                 return py::array_t<long>(nb_dim, c_dim);
                               },
                               "TODO") // TODO do the documentation...

        // int getNbElem()
        .def_property_readonly("nbElem", &Class::getNbElem,
                               "TODO") // TODO do the documentation...
    ;
    // MAGMA functions

    // template<class T>
    // int carma_svd(carma_obj<T> *imat, carma_obj<T> *eigenvals,
    //               carma_obj<T> *mod2act, carma_obj<T> *mes2mod);
    mod.def(appendName<T>("svd_cpu_").data(), py::overload_cast<Class *,
                  Class *, Class *, Class *>(&carma_svd_cpu<T>));

    // TODO after carma_host_obj
    // template<class T>
    // int carma_syevd(char jobz, carma_obj<T> *mat, carma_host_obj<T>
    // *eigenvals);
    // mod.def(appendName<T>("syevd_").data(), &carma_syevd<T>);
    mod.def(appendName<T>("syevd_cpu_").data(), py::overload_cast<char, Class *, Class *>(&carma_syevd_cpu<T>));

    // template<class T, int method>
    // int carma_syevd(char jobz, carma_obj<T> *mat, carma_host_obj<T>
    // *eigenvals); template<class T> int carma_syevd_m(long ngpu, char jobz,
    // long N, T *mat, T *eigenvals); template<class T> int carma_syevd_m(long
    // ngpu, char jobz, carma_host_obj<T> *mat,
    //                   carma_host_obj<T> *eigenvals);
    // template<class T>
    // int carma_syevd_m(long ngpu, char jobz, carma_host_obj<T> *mat,
    //                   carma_host_obj<T> *eigenvals, carma_host_obj<T> *U);
    // template<class T>
    // int carma_getri(carma_obj<T> *d_iA);
    mod.def(appendName<T>("getri_cpu_").data(), py::overload_cast<Class *>(&carma_getri_cpu<T>));

    // template<class T>
    // int carma_potri(carma_obj<T> *d_iA);
    mod.def(appendName<T>("potri_cpu_").data(), py::overload_cast<Class *>(&carma_potri_cpu<T>));

    // TODO after carma_host_obj
    // template<class T>
    // int carma_potri_m(long num_gpus, carma_host_obj<T> *h_A, carma_obj<T>
    // *d_iA);

    // MAGMA functions (direct access)
    // template<class T>
    // int carma_syevd(char jobz, long N, T *mat, T *eigenvals);
    // template<class T, int method>
    // int carma_syevd(char jobz, long N, T *mat, T *eigenvals);
    // template<class T>
    // int carma_syevd_m(long ngpu, char jobz, long N, T *mat, T *eigenvals);
    // template<class T>
    // int carma_potri_m(long num_gpus, long N, T *h_A, T *d_iA);

    // CULA functions
    // template<class T>
    // int carma_cula_svd(carma_obj<T> *imat, carma_obj<T> *eigenvals,
    //                   carma_obj<T> *mod2act, carma_obj<T> *mes2mod);

    // int snapTransformSize(unsigned int dataSize);
  }
};
#endif