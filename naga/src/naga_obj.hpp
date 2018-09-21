#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <carma.h>

namespace py = pybind11;

template <typename T>
void declare_naga_obj(py::module &mod, std::string suffix)
{

  using Class = carma_obj<T>;

  py::class_<Class>(mod, ("naga_obj_" + suffix).c_str(), py::buffer_protocol())
      .def(py::init([](carma_context &c, const py::array_t<T, py::array::f_style | py::array::forcecast> &data) {
        int ndim = data.ndim() + 1;
        std::vector<long> data_dims(ndim);
        data_dims[0] = data.ndim();
        copy(data.shape(), data.shape()+data.ndim(), begin(data_dims) + 1);
        return std::unique_ptr<Class>(new Class(&c, data_dims.data(), (const T *)data.data()));
      }), "TODO", // TODO do the documentation...
      py::arg("context").none(false),
      py::arg("h_data").none(false))

      .def(py::init([](carma_context &c, const Class &data) {
        return std::unique_ptr<Class>(new Class(&c, &data));
      }), "TODO", // TODO do the documentation...
      py::arg("context").none(false),
      py::arg("d_data").none(false))

      .def_buffer([](Class &frame) -> py::buffer_info {
        frame.sync_h_data();

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
        for (ssize_t dim(0); dim < dims[0]; ++dim)
        {
          shape[dim] = dims[dim + 1];
          strides[dim] = stride;
          stride *= shape[dim];
        }

        return py::buffer_info(frame.get_h_data(),
                               sizeof(T),
                               py::format_descriptor<T>::format(),
                               dims[0],
                               shape,
                               strides);
      })

      .def("__repr__", &Class::toString)

      // int get_nbStreams()
      .def_property_readonly("nbStreams", &Class::get_nbStreams, "TODO") // TODO do the documentation...
      // int add_stream()
      .def("add_stream", (int (Class::*)()) & Class::add_stream, "TODO") // TODO do the documentation...
      // int add_stream(int nb)
      .def("add_stream", (int (Class::*)(int)) & Class::add_stream, "TODO", py::arg("np")) // TODO do the documentation...
      // int del_stream()
      .def("del_stream", (int (Class::*)()) & Class::del_stream, "TODO") // TODO do the documentation...
      // int del_stream(int nb)
      .def("del_stream", (int (Class::*)(int)) & Class::del_stream, "TODO", py::arg("np")) // TODO do the documentation...
      // int wait_stream(int stream)
      .def("wait_stream", &Class::wait_stream, "TODO", py::arg("steam")) // TODO do the documentation...
      // int wait_all_streams()
      .def("wait_all_streams", &Class::wait_all_streams, "TODO") // TODO do the documentation...

      // const long *getDims()
      .def_property_readonly("shape", [](Class &frame) -> py::array_t<long> {
        long nb_dim = frame.getDims(0);
        const long *c_dim = frame.getDims()+1;
        return py::array_t<long>(nb_dim, c_dim);

      }, "TODO") // TODO do the documentation...

      // int getNbElem()
      .def_property_readonly("nbElem", &Class::getNbElem, "TODO") // TODO do the documentation...
      // carma_context* getContext()
      .def_property_readonly("context", &Class::getContext, "TODO") // TODO do the documentation...
      // int getDevice()
      .def_property_readonly("device", &Class::getDevice, "TODO") // TODO do the documentation...

      // int host2device(T_data *data);
      .def("host2device", [](Class &c, py::array_t<T, py::array::f_style | py::array::forcecast> &data) {
        c.host2device((const T*)data.data());
      }, "TODO", py::arg("data").none(false)) // TODO do the documentation...
      // int device2host(T_data *data);
      .def("device2host", [](Class &c, py::array_t<T, py::array::f_style | py::array::forcecast> &data) {
        c.device2host((T*)data.mutable_data());
      }, "TODO", py::arg("data").none(false)) // TODO do the documentation...

      // int copyInto(T_data *data, int nb_elem);
      .def("copyInto", [](Class &src, Class &dest, long nb_elem) {
        if(nb_elem < 0){
          nb_elem = src.getNbElem();
        }
        src.copyInto(dest, nb_elem);
      }, "TODO", py::arg("dest"), py::arg("nb_elem")=-1) // TODO do the documentation...
      // int copyFrom(T_data *data, int nb_elem);
      .def("copyFrom", [](Class &dest, Class &src, long nb_elem) {
        if(nb_elem < 0){
          nb_elem = dest.getNbElem();
        }
        dest.copyFrom(src, nb_elem);
      }, "TODO", py::arg("data"), py::arg("nb_elem")=-1) // TODO do the documentation...

      // inline int reset()
      .def("reset", &Class::reset, "TODO") // TODO do the documentation...

      /**< sum */
      // T_data sum();
      .def("sum", &Class::sum, "TODO") // TODO do the documentation...
      // void clip(T_data min, T_data max);
      .def("clip", &Class::clip, "TODO", py::arg("data_min").none(false), py::arg("data_max").none(false)) // TODO do the documentation...

      // /**< transpose */
      // int transpose(carma_obj<T_data> *source);
      .def("transpose", &Class::transpose, "TODO", py::arg("source").none(false)) // TODO do the documentation...
      // //carma_obj<T_data>& operator= (const carma_obj<T_data>& obj);

      // /**< Cublas V2 */
      // int imax(int incx);
      .def("aimax", &Class::aimax, "TODO", py::arg("incx")=1) // TODO do the documentation...
      // int imin(int incx);
      .def("aimin", &Class::aimin, "TODO", py::arg("incx")=1) // TODO do the documentation...
      // T_data asum(int incx);
      .def("asum", &Class::asum, "TODO", py::arg("incx")=1) // TODO do the documentation...
      // T_data nrm2(int incx);
      .def("nrm2", &Class::nrm2, "TODO", py::arg("incx")=1) // TODO do the documentation...
      // T_data dot(carma_obj<T_data> *source, int incx, int incy);
      .def("dot", &Class::dot, "TODO", py::arg("source").none(false), py::arg("incx")=1, py::arg("incy")=1) // TODO do the documentation...
      // void scale(T_data alpha, int incx);
      .def("scale", &Class::scale, "TODO", py::arg("scale").none(false), py::arg("incx")=1) // TODO do the documentation...
      // void swap(carma_obj<T_data> *source, int incx, int incy);
      .def("swap", &Class::swap, "TODO", py::arg("source").none(false), py::arg("incx")=1, py::arg("incy")=1) // TODO do the documentation...
      // void copy(carma_obj<T_data> *source, int incx, int incy);
      .def("copy", &Class::copy, "TODO", py::arg("source").none(false), py::arg("incx")=1, py::arg("incy")=1) // TODO do the documentation...
      // void axpy(T_data alpha, carma_obj<T_data> *source, int incx, int incy);
      .def("axpy", &Class::axpy, "TODO", py::arg("alpha"), py::arg("source").none(false), py::arg("incx")=1, py::arg("incy")=1) // TODO do the documentation...
      // void rot(carma_obj<T_data> *source, int incx, int incy, T_data sc,
      //          T_data ss);
      .def("rot", &Class::rot, "TODO") // TODO do the documentation...

      // void gemv(char trans, T_data alpha, carma_obj<T_data> *matA, int lda,
      //           carma_obj<T_data> *vectx, int incx, T_data beta, int incy);
      .def("gemv", [](Class &mat, Class &vectx, T alpha, char op, Class *vecty, T beta) {
        if(vecty == nullptr){
          long dims[] = {1, 0};
          if (op == 'N' || op == 'n')
          {
            dims[1] = mat.getDims(1);
          }
          else
          {
            dims[1] = mat.getDims(2);
          }
          vecty = new Class(mat.getContext(), dims);
        }
        vecty->gemv(op, alpha, &mat, mat.getDims(1), &vectx, 1, beta, 1);
        return vecty;
      },
           "this method performs one of the matrix‐vector operations vecty = alpha * op(mat) * vectx + beta * vecty",
           py::arg("vectx"), py::arg("alpha") = 1, py::arg("op") = 'N', py::arg("vecty")=nullptr, py::arg("beta") = 0) // &Class::gemv)
      // void ger(T_data alpha, carma_obj<T_data> *vectx, int incx,
      //          carma_obj<T_data> *vecty, int incy, int lda);
      .def("ger", [](Class &vectx, Class &vecty, Class *mat, T alpha) {
        std::unique_ptr<Class> ptr_res;
        if (mat == nullptr)
        {
          long dims[] = {2, vectx.getNbElem(), vecty.getNbElem()};
          mat = new Class(vectx.getContext(), dims);
        }
        mat->ger(alpha, &vectx, 1, &vecty, 1, vectx.getNbElem());
        return mat;
      },
           "this method performs the symmetric rank 1 operation A = alpha * x * y T + A",
           py::arg("vecty"), py::arg("mat") = nullptr, py::arg("alpha") = 1) // &Class::ger)
      // void symv(char uplo, T_data alpha, carma_obj<T_data> *matA,
      //           int lda, carma_obj<T_data> *vectx, int incx, T_data beta, int incy);
      .def("symv", [](Class &mat, Class &vectx, T alpha, char uplo, Class *vecty, T beta) {
        int lda = mat.getDims(2);
        if(vecty==nullptr) {
          long dims[] = {1, lda};
          vecty = new Class(mat.getContext(), dims);
        }
        vecty->symv(uplo, alpha, &mat, lda, &vectx, 1, beta, 1);
        return vecty;
      },
           "this method performs one of the matrix‐vector operations vecty = alpha * mat * vectx + beta * vecty",
           py::arg("vectx"), py::arg("alpha") = 1, py::arg("uplo") = 'l', py::arg("vecty")=nullptr, py::arg("beta") = 0) // &Class::gemv)
      // void gemm(char transa, char transb, T_data alpha, carma_obj<T_data> *matA,
      //           int lda, carma_obj<T_data> *matB, int ldb, T_data beta, int ldc);
      .def("gemm",  [](Class &matA, Class &matB, char op_a, char op_b, T alpha, Class *matC, T beta
      ) /*-> std::unique_ptr<Class>*/  {

        int lda, ldb, ldc, m, n, k;
        if (op_a == 'N' || op_a == 'n')
        {
          m = matA.getDims(1);
          k = matA.getDims(2);
        }
        else
        {
          m = matA.getDims(2);
          k = matA.getDims(1);
        }

        if (op_b == 'N' || op_b == 'n')
        {
          k = matB.getDims(1);
          n = matB.getDims(2);
        }
        else
        {
          k = matB.getDims(2);
          n = matB.getDims(1);
        }

        if (matC == nullptr)
        {
          long dims[] = {2, m, n};
          matC = new Class(matA.getContext(), dims);
        }

        carma_gemm<T>(matA.getContext()->get_cublasHandle(), op_a, op_b, m, n, k, alpha, matA, matA.getDims(1), matB, matB.getDims(1), beta, *matC, matC->getDims(1));
        return matC;
      },
           "this method performs one of the matrix‐marix operations matC = alpha * op_a(matA) * op_b(matB) + beta * matC",
           py::arg("matB"), py::arg("op_a")='N', py::arg("op_b")="N", py::arg("alpha")=1, py::arg("matC")=nullptr, py::arg("beta") = 0)

      // void symm(char side, char uplo, T_data alpha,
      //           carma_obj<T_data> *matA, int lda, carma_obj<T_data> *matB, int ldb,
      //           T_data beta, int ldc);
      .def("symm", [](Class &matA, Class &matB, T alpha, Class *matC, T beta, char side, char uplo) {
        if (matC == nullptr)
        {
          long dims[] = {2, matB.getDims(1),  matB.getDims(2)};
          matC = new Class(matA.getContext(), dims);
        }
        carma_symm<T>(matA.getContext()->get_cublasHandle(), side, uplo,  matB.getDims(1),  matB.getDims(2), alpha, matA, matA.getDims(1), matB, matB.getDims(1), beta, *matC, matC->getDims(1));
        return matC;
        // matA.symm(uplo, alpha, &matB, lda, &vectx, 1, beta, 1);
      },
           "this method performs one of the matrix‐marix operations matC = alpha * matA * matB + beta * C",
           py::arg("matB"), py::arg("alpha")=1, py::arg("matC")=nullptr, py::arg("beta")=0, py::arg("side")="l", py::arg("uplo")="u")

/*
template <class T_data>
cublasStatus_t carma_symm(cublasHandle_t cublas_handle, char side,
                          char uplo, int m, int n, T_data alpha,
                          T_data *matA, int lda, T_data *matB, int ldb,
                          T_data beta, T_data *matC, int ldc);
*/

      // void syrk(char uplo, char transa, T_data alpha,
      //           carma_obj<T_data> *matA, int lda, T_data beta, int ldc);
      .def("syrk", [](Class &matA, char fill, char op, T alpha, Class *matC, T beta) {
        int n,k;
        if (op == 'N' || op == 'n')
        {
          n = matA.getDims(1);
          k = matA.getDims(2);
        }
        else
        {
          n = matA.getDims(2);
          k = matA.getDims(1);
        }
        if(matC == nullptr){
          long dims[] = {2, n, n};
          matC = new Class(matA.getContext(), dims);
        }
        carma_syrk<T>(matA.getContext()->get_cublasHandle(), fill, op,  n,  k, alpha, matA, matA.getDims(1), beta, *matC, matC->getDims(1));
        return matC;
      },
           "this method performs the symmetric rank- k update",
           py::arg("fill")="U", py::arg("op")='N', py::arg("alpha")=1, py::arg("matC")=nullptr, py::arg("beta")=0)
      // void syrkx(char uplo, char transa, T_data alpha,
      //            carma_obj<T_data> *matA, int lda, carma_obj<T_data> *matB, int ldb,
      //            T_data beta, int ldc);
      .def("syrkx", [](Class &matA, Class &matB, char fill, char op, T alpha, Class *matC, T beta) {
        int n,k;
        if (op == 'N' || op == 'n')
        {
          n = matA.getDims(1);
          k = matA.getDims(2);
        }
        else
        {
          n = matA.getDims(2);
          k = matA.getDims(1);
        }
        if(matC == nullptr){
          long dims[] = {2, n, n};
          matC = new Class(matA.getContext(), dims);
        }
        carma_syrkx<T>(matA.getContext()->get_cublasHandle(), fill, op,  n,  k, alpha, matA, matA.getDims(1), matB, matB.getDims(1), beta, *matC, matC->getDims(1));
        return matC;
      },
           "this method performs the symmetric rank- k update",
           py::arg("matB"), py::arg("fill")="U", py::arg("op")='N', py::arg("alpha")=1, py::arg("matC")=nullptr, py::arg("beta")=0)
      // void geam(char transa, char transb, T_data alpha, carma_obj<T_data> *matA,
      //           int lda, T_data beta, carma_obj<T_data> *matB, int ldb, int ldc);
      .def("geam", [](Class &matA, Class &matB, char opA, char opB, T alpha, Class *matC, T beta) {
        int m,n;
        if (opA == 'N' || opA == 'n')
        {
          m = matA.getDims(1);
          n = matA.getDims(2);
        }
        else
        {
          m = matA.getDims(2);
          n = matA.getDims(1);
        }
        if(matC == nullptr){
          long dims[] = {2, m, n};
          matC = new Class(matA.getContext(), dims);
        }
        carma_geam<T>(matA.getContext()->get_cublasHandle(), opA, opB,  m,  n, alpha, matA, matA.getDims(1), beta, matB, matB.getDims(1), *matC, matC->getDims(1));
        return matC;
      },
           "this method performs the symmetric rank- k update",
           py::arg("matB"), py::arg("opA")='N', py::arg("opB")='N', py::arg("alpha")=1, py::arg("matC")=nullptr, py::arg("beta")=0)
      .def("dgmm", [](Class &matA, Class &vectX, T alpha, char side, Class *matC, int incx) {
        if(matC == nullptr){
          long dims[] = {2, matA.getDims(1),  matA.getDims(2)};
          matC = new Class(matA.getContext(), dims);
        }
        carma_dgmm<T>(matA.getContext()->get_cublasHandle(), side,  matA.getDims(1),  matA.getDims(2), matA, matA.getDims(1), vectX, incx, *matC, matC->getDims(1));
        return matC;
      },
           "this method performs one of the matrix‐marix operations matC = diag(vectX)*matA if side='l'",
           py::arg("vectX"), py::arg("alpha")=1, py::arg("side")="r", py::arg("matC")=nullptr, py::arg("incx")=1)

      // /**< Curand */
      .def("is_rng_init", &Class::is_rng_init)
      // int init_prng();
      .def("init_prng", (int (Class::*)()) & Class::init_prng)
      // int init_prng(long seed);
      .def("init_prng", (int (Class::*)(long)) & Class::init_prng)
      // int destroy_prng();
      .def("destroy_prng", &Class::destroy_prng)
      // int prng(T_data *output, char gtype, float alpha, float beta);
      .def("prng", (int (Class::*)(T *, char, float, float)) & Class::prng)
      // int prng(T_data *output, char gtype, float alpha);
      .def("prng", (int (Class::*)(T *, char, float)) & Class::prng)
      // int prng(char gtype, float alpha, float beta);
      .def("prng", (int (Class::*)(char, float, float)) & Class::prng)
      // int prng(char gtype, float alpha);
      .def("prng", (int (Class::*)(char, float)) & Class::prng)
      // int prng(char gtype);
      .def("prng", (int (Class::*)(char)) & Class::prng)

      .def("random", [](Class &data, int seed, char gtype) {
        data.init_prng(seed);
        data.prng(gtype);
      },
           py::arg("seed") = 1234, py::arg("j") = 'U')

      .def("random_host", [](Class &data, int seed, char gtype) {
        data.init_prng_host(seed);
        data.prng_host(gtype);
      },
           py::arg("seed") = 1234, py::arg("j") = 'U')

      // int prng_montagn( float init_montagn );
      .def("prng_montagn", &Class::prng_montagn)

      // int init_prng_host(int seed);
      .def("init_prng_host", (int (Class::*)(int)) & Class::init_prng_host)
      // int prng_host(char gtype);
      .def("prng_host", (int (Class::*)(char)) & Class::prng_host)
      // int prng_host(char gtype, T_data stddev);
      .def("prng_host", (int (Class::*)(char, T)) & Class::prng_host)
      // int prng_host(char gtype, T_data stddev, T_data alpha);
      .def("prng_host", (int (Class::*)(char, T, T)) & Class::prng_host)
      // int destroy_prng_host();
      .def("destroy_prng_host", &Class::destroy_prng_host)

      ;
  // CU functions clip
  // template<class T_data>
  // void clip_array(T_data *d_data, T_data min, T_data max, int N, carma_device *device);

  // CU functions sum
  // template<class T_data>
  // void reduce(int size, int threads, int blocks, T_data *d_idata,
  //             T_data *d_odata);
  // template<class T_data>
  // T_data reduce(T_data * data, int N);

  // CU functions transpose
  // template<class T_data>
  // int transposeCU(T_data *d_idata, T_data *d_odata, long N1, long N2);

  // CU functions generic
  // template<class T_data>
  // int launch_generic1d(T_data *d_idata, T_data *d_odata, int N,
  //                     carma_device *device);
  // template<class T_data>
  // int launch_generic2d(T_data *d_odata, T_data *d_idata, int N1, int N2);

  // CU functions curand
  // int carma_prng_init(int *seed, const int nThreads, const int nBlocks,
  //                     curandState *state);
  // template<class T>
  // int carma_prng_cu(T *results, const int nThreads, const int nBlocks,
  //                   curandState *state, char gtype, int n, float alpha, float beta);
  // template<class T>
  // int carma_curand_montagn(curandState *state, T *d_odata, int N, carma_device *device);

  // CU functions fft
  // template<class T_in, class T_out>
  // cufftType carma_select_plan();
  // template<class T_in, class T_out>
  // void carma_initfft(const long *dims_data, cufftHandle *plan, cufftType tPlan);
  // template<class T_in, class T_out>
  // int carma_fft(T_in *input, T_out *output, int dir, cufftHandle plan);

  // CU functions generic
  // template<class T_data>
  // int fillindex(T_data *d_odata, T_data *d_idata, int *indx, int N,
  //               carma_device *device);
  // template<class T_data>
  // int fillvalues(T_data *d_odata, T_data val, int N,
  //               carma_device *device);
  // template<class T>
  // int getarray2d(T *d_odata, T *d_idata, int x0, int Ncol, int NC, int N,
  //               carma_device *device);
  // template<class T>
  // int fillarray2d(T *d_odata, T *d_idata, int x0, int Ncol, int NC, int N,
  //                 carma_device *device);
  // template<class T>
  // int fillarray2d2(T *d_odata, T *d_idata, int x0, int Ncol, int NC, int N,
  //                 carma_device *device);
  // template<class T>
  // int fill_sym_matrix(char src_uplo, T *d_data, int Ncol, int N,
  //                     carma_device *device);
  // template<class T>
  // int carma_plus(T *d_odata, T elpha, int N, carma_device *device);
  // template<class T>
  // int carma_plusai(T *d_odata, T *i_data, int i, int sgn, int N,
  //                 carma_device *device);

  // CU functions fftconv
  // int fftconv_unpad(float *d_odata, float *d_idata, int fftW, int dataH,
  //                   int dataW, int N, int n, int nim);
  // int carma_initfftconv(caObjS *data_in, caObjS *kernel_in, caObjS *padded_data,
  // caObjC *padded_spectrum, int kernelY, int kernelX);

  // CPP functions fftconv
  // int carma_fftconv(caObjS *data_out, caObjS *padded_data,
  //                   caObjC *padded_spectrum, int kernelY, int kernelX);

  // MAGMA functions

  // template<class T>
  // int carma_svd(carma_obj<T> *imat, carma_obj<T> *eigenvals,
  //               carma_obj<T> *mod2act, carma_obj<T> *mes2mod);
  mod.def(("naga_svd_" + suffix).c_str(), &carma_svd<T>);

  // TODO after carma_host_obj
  // template<class T>
  // int carma_syevd(char jobz, carma_obj<T> *mat, carma_host_obj<T> *eigenvals);
  // mod.def( ("naga_syevd_" + suffix).c_str(), &carma_syevd<T>);

  // template<class T, int method>
  // int carma_syevd(char jobz, carma_obj<T> *mat, carma_host_obj<T> *eigenvals);
  // template<class T>
  // int carma_syevd_m(long ngpu, char jobz, long N, T *mat, T *eigenvals);
  // template<class T>
  // int carma_syevd_m(long ngpu, char jobz, carma_host_obj<T> *mat,
  //                   carma_host_obj<T> *eigenvals);
  // template<class T>
  // int carma_syevd_m(long ngpu, char jobz, carma_host_obj<T> *mat,
  //                   carma_host_obj<T> *eigenvals, carma_host_obj<T> *U);
  // template<class T>
  // int carma_getri(carma_obj<T> *d_iA);
  mod.def(("naga_getri_" + suffix).c_str(), &carma_getri<T>);

  // template<class T>
  // int carma_potri(carma_obj<T> *d_iA);
  mod.def(("naga_potri_" + suffix).c_str(), &carma_potri<T>);

  // TODO after carma_host_obj
  // template<class T>
  // int carma_potri_m(long num_gpus, carma_host_obj<T> *h_A, carma_obj<T> *d_iA);

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