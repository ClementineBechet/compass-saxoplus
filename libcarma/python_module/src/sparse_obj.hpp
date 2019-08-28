#ifndef _WRAP_SPARSE_OBJ_H_
#define _WRAP_SPARSE_OBJ_H_

#include "declare_name.hpp"

#include <carma.h>

#include <wyrm>

#include <carma_sparse_obj.h>

#include <type_list.hpp>

namespace py = pybind11;

struct CarmaSparseObjInterfacer {
  template <typename T> static void call(py::module &mod) {
    auto name = appendName<T>("sparse_obj_");
    using Class = carma_sparse_obj<T>;

    py::class_<Class>(mod, name.data(), py::buffer_protocol())
    .def("get_csr",[](Class &frame){
        py::object CSR = py::module::import("scipy.sparse").attr("csr_matrix");
        int dim1 = frame.getDims(1);
        int dim2 = frame.getDims(2);
        int nnz = frame.nz_elem;

        std::vector<int> rowind = std::vector<int>(dim1 + 1);
        std::vector<int> colind = std::vector<int>(nnz);
        std::vector<T> data = std::vector<T>(nnz);

        frame.sparse_to_host(rowind.data(), colind.data(), data.data());
        py::tuple csrMat = py::make_tuple(py::array(data.size(),data.data()), py::array(colind.size(),colind.data()),py::array(rowind.size(), rowind.data()));
        py::tuple shape = py::make_tuple(dim1,dim2);


        return CSR(csrMat,py::arg("shape")=shape);
        });
}
};
#endif
