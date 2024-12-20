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

//! \file      carma_sparse_host_obj.cpp
//! \ingroup   libcarma
//! \class     CarmaSparseHostObj
//! \brief     this class provides wrappers to the generic carma sparse host object
//! \author    COSMIC Team <https://github.com/COSMIC-RTC/compass>
//! \date      2022/01/24


#include "carma_sparse_host_obj.hpp"
#include <algorithm>
#include "carma_sparse_obj.hpp"

template <class T_data>
CarmaSparseHostObj<T_data>::CarmaSparseHostObj() {
  _create(0, 0, 0);
}

template <class T_data>
CarmaSparseHostObj<T_data>::~CarmaSparseHostObj() {
  _clear();
}

template <class T_data>
CarmaSparseHostObj<T_data>::CarmaSparseHostObj(
    CarmaSparseHostObj<T_data> &M) {
  _create(0, 0, 0);
  this->operator=(M);
}

template <class T_data>
CarmaSparseHostObj<T_data>::CarmaSparseHostObj(
    CarmaSparseObj<T_data> &M) {
  _create(M.nz_elem, M.get_dims(1), M.get_dims(2));

  cudaMemcpy(h_data, M.d_data, nz_elem * sizeof(T_data),
             cudaMemcpyDeviceToHost);
  cudaMemcpy(rowind, M.d_rowind, (dims_data[1] + 1) * sizeof(int32_t),
             cudaMemcpyDeviceToHost);
  cudaMemcpy(colind, M.d_colind, nz_elem * sizeof(int32_t), cudaMemcpyDeviceToHost);

  major_dim = M.get_major_dim();
}

template <class T_data>
CarmaSparseHostObj<T_data>::CarmaSparseHostObj(const int64_t *dims,
                                                     T_data *M, char order) {
  _create(0, 0, 0);
  init_from_matrix(dims, M, order);
}

template <class T_data>
void CarmaSparseHostObj<T_data>::resize(int32_t nnz_, int32_t dim1_, int32_t dim2_) {
  if (nz_elem != nnz_) {
    _clear();
    _create(nnz_, dim1_, dim2_);
  } else {
    dims_data[0] = 2;
    dims_data[1] = dim1_;
    dims_data[2] = dim2_;
    major_dim = 'U';
  }
}
//
template <class T_data>
void CarmaSparseHostObj<T_data>::operator=(
    CarmaSparseHostObj<T_data> &M) {
  resize(M.nz_elem, M.get_dims(1), M.get_dims(2));
  memcpy(h_data, M.h_data, sizeof(T_data) * nz_elem);
  memcpy(rowind, M.rowind, sizeof(int32_t) * (dims_data[1] + 1));
  memcpy(colind, M.colind, sizeof(int32_t) * nz_elem);

  major_dim = M.major_dim;
}
//
template <class T_data>
void CarmaSparseHostObj<T_data>::operator=(CarmaSparseObj<T_data> &M) {
  resize(M.nz_elem, M.get_dims(1), M.get_dims(2));
  cudaMemcpy(h_data, M.d_data, nz_elem * sizeof(T_data),
             cudaMemcpyDeviceToHost);
  cudaMemcpy(rowind, M.d_rowind, (dims_data[1] + 1) * sizeof(int32_t),
             cudaMemcpyDeviceToHost);
  cudaMemcpy(colind, M.d_colind, nz_elem * sizeof(int32_t), cudaMemcpyDeviceToHost);

  major_dim = M.major_dim;
}

template <class T_data>
void CarmaSparseHostObj<T_data>::init_from_matrix(const int64_t *dims,
                                                     T_data *M, char major_dim) {
  int64_t nb_el = dims[1] * dims[2];
  T_data *valB = M;
  int64_t cmpt = 0;
  for (int64_t i = 0; i < nb_el; i++) {
    if (*valB != 0) {
      cmpt++;
    }
    valB++;
  }

  resize(cmpt, dims[1], dims[2]);
  int32_t *rowcoo = new int32_t[cmpt];

  int32_t *dim1, *dim2, ld;
  if (major_dim == 'R') {
    dim1 = rowcoo;
    dim2 = this->colind;
    ld = dims_data[1];
  } else {
    dim1 = this->colind;
    dim2 = rowcoo;
    ld = dims_data[2];
  }
  valB = M;
  int64_t index = 0;
  for (int64_t i = 0; (index < cmpt) || (i < nb_el); i++) {
    if (*valB != 0) {
      dim1[index] = i / ld;
      dim2[index] = i - dim1[index] * ld;
      h_data[index++] = *valB;
    }
    valB++;
  }

  if (major_dim == 'C') {
    // sort rows
    std::vector<std::pair<int32_t, std::pair<int32_t, T_data> > > sM(nz_elem);
    // sM[i].first         -> rowind
    // sM[i].second.first  -> colind
    // sM[i].second.second -> value
    for (int32_t i = 0; i < nz_elem; i++) {
      sM[i].first = rowcoo[i];
      sM[i].second.first = colind[i];
      sM[i].second.second = h_data[i];
    }
    sort(sM.begin(), sM.end());
    for (int32_t i = 0; i < nz_elem; i++) {
      rowcoo[i] = sM[i].first;
      colind[i] = sM[i].second.first;
      h_data[i] = sM[i].second.second;
    }
  }

  // coo2csr
  rowind[0] = 0;
  int64_t csr_index = 0;
  index = 0;
  while (csr_index < ld + 1) {
    while (rowcoo[index] == csr_index) {
      index++;
    }
    rowind[++csr_index] = index;
  }

  delete[] rowcoo;
  //  DEBUG_TRACE("h_data:\n");
  //  for(index=0; index<nz_elem; index++){
  //    fprintf(stderr, "%f ", h_data[index]);
  //  } fprintf(stderr, "\n");
  //  DEBUG_TRACE("rowind:\n");
  //  for(index=0; index<ld+1; index++){
  //    fprintf(stderr, "%d ", rowind[index]);
  //  } fprintf(stderr, "\n");
  //  DEBUG_TRACE("colind:\n");
  //  for(index=0; index<nz_elem; index++){
  //    fprintf(stderr, "%d ", colind[index]);
  //  } fprintf(stderr, "\n");

  this->major_dim = major_dim;
}

template <class T_data>
void CarmaSparseHostObj<T_data>::copy_into_matrix(T_data *M, char major_dim) {
  int32_t ld;
  if (major_dim == 'R') {
    ld = dims_data[1];
  } else {
    ld = dims_data[2];
  }
  T_data *val = this->h_data;
  int32_t index = 0;
  for (int32_t i = 0; i < dims_data[1]; i++) {
    while (index < rowind[i + 1]) {
      if (major_dim == 'R') {
        M[i * ld + colind[index]] = *val;
      } else {
        M[i + ld * colind[index]] = *val;
      }

      // DEBUG_TRACE("%d %d %d %d\n",i,index, rowind[i+1],
      // rowind[i]+ld*colind[index]);
      index++;
      val++;
    }
  }
}

//
template <class T_data>
void CarmaSparseHostObj<T_data>::_create(int32_t nnz_, int32_t dim1_, int32_t dim2_) {
  nz_elem = nnz_;
  dims_data[0] = 2;
  dims_data[1] = dim1_;
  dims_data[2] = dim2_;
  h_data = new T_data[nz_elem];
  rowind = new int32_t[dim1_ + 1];
  colind = new int32_t[nz_elem];
  major_dim = 'U';
}

//
template <class T_data>
void CarmaSparseHostObj<T_data>::_clear() {
  if (h_data == NULL || rowind == NULL || colind == NULL) {
    std::cerr << "Error | CarmaSparseHostObj<T_data>::_clear | double clear"
              << std::endl;
    throw "Error | CarmaSparseHostObj<T_data>::_clear | double clear";
    // exit(EXIT_FAILURE);
  }
  delete[] h_data;
  delete[] rowind;
  delete[] colind;
  h_data = NULL;
  rowind = NULL;
  colind = NULL;
  nz_elem = 0;
  dims_data[0] = 2;
  dims_data[1] = 0;
  dims_data[2] = 0;
  major_dim = 'U';
}
//
template <class T_data>
void carma_gemv(T_data alpha, CarmaSparseHostObj<T_data> *A,
                CarmaHostObj<T_data> *x, T_data betta,
                CarmaHostObj<T_data> *y,
                void (*ptr_coomv)(char *transa, int64_t *m, int64_t *k, T_data *alpha,
                                  char *matdescra, T_data *val, int32_t *rowind,
                                  int32_t *colind, int32_t *nnz, T_data *x,
                                  T_data *beta, T_data *y)) {
  if (A->get_dims(2) != x->get_dims(1) || A->get_dims(1) != y->get_dims(1)) {
    std::cerr << "Error | kp_cscmv | diminsion problem" << std::endl;
    throw "Error | kp_cscmv | diminsion problem";
    // exit(EXIT_FAILURE);
  }
  //   A.check();
  int64_t dim1 = A->get_dims(1);
  int64_t dim2 = A->get_dims(2);
  int32_t nz_elem = A->nz_elem;

  char transa[] = "N";
  char matdescra[] = "GFFFFFFFF";
  ptr_coomv(transa, &dim1, &dim2, &alpha, matdescra, A->get_data(), A->rowind,
            A->colind, &nz_elem, x->get_data(), &betta, y->get_data());
  //   mkl_dcscmv("N", &A.dim1, &A.dim2, &alpha, "GFFFFFFF",
  //        A.values, A.rows, A.pointerB, A.pointerB + 1,
  //        x.d,  &betta, y.d);
}

template <class T_data>
void carma_gemm(char op_A, T_data alpha, CarmaSparseHostObj<T_data> *A,
                CarmaHostObj<T_data> *B, T_data betta,
                CarmaHostObj<T_data> *C) {
  //   A.check();
  /* FIXME: à coder sans MKL ou pas...
   int64_t dimA1=A->get_dims(1);
   int64_t dimA2=A->get_dims(2);
   int64_t dimB1=B->get_dims(1);
   int64_t dimB2=B->get_dims(2);
   int64_t dimC1=C->get_dims(1);
   int64_t dimC2=C->get_dims(2);
   int64_t nz_elem=A->nz_elem;

   mkl_dcoomm(&op_A, &dimA1, &dimC2, &dimA2, &alpha, "GFFFFFFF", A->get_data(),
   A->rowind, A->colind, &nz_elem, B->get_data(), &dimB1, &betta, C->get_data(),
   &dimC1);
   */

  //   mkl_dcscmm("N", &A.dim1, &C.dim2,  &A.dim2, &alpha, "GCCCCCCC",
  //        A.values, A.rows, A.pointerB, A.pointerB+1,
  //        B.d, &B.tda,  &betta, C.d, & C.tda);
}

template <class T_data>
void CarmaSparseHostObj<T_data>::resize2row_major() {
  int32_t i;
  int32_t indice;
  std::vector<std::vector<std::pair<int32_t, int32_t> > > position(
      dims_data[1], std::vector<std::pair<int32_t, int32_t> >());
  int32_t *rowind2 = new int32_t[nz_elem];
  int32_t *colind2 = new int32_t[nz_elem];
  T_data *values2 = new T_data[nz_elem];

  // cout<<"dim1="<<this->dim1<<" dim2="<<this->dim2<<" nnz="<<this->nnz<<endl;
  // cout<<"i="<<172248<<" rowind[i]="<<rowind[172248]<<"
  // colind[i]="<<colind[172248]<<endl;

  for (i = 0; i < nz_elem; i++) {
    // cout<<"i="<<i<<" rowind[i]="<<rowind[i]-1<<"
    // colind[i]="<<colind[i]-1<<endl;
    position[rowind[i]].push_back(std::make_pair(colind[i], i));
  }

  indice = 0;
  for (i = 0; i < dims_data[1]; i++) {
    if (position[i].size() > 0) {
      sort(position[i].begin(), position[i].end());
      for (size_t j = 0; j < position[i].size(); j++) {
        rowind2[indice] = rowind[(position[i][j]).second];
        colind2[indice] = colind[(position[i][j]).second];
        values2[indice] = h_data[(position[i][j]).second];
        indice++;
      }
    }
  }
  if (indice != nz_elem) {
    std::cerr << "Erreur | CarmaSparseHostObj<T_data>::resize2row_major | "
                 "erreur lors de la conversion."
              << std::endl;
    delete[] rowind2;
    delete[] colind2;
    delete[] values2;
    rowind2 = nullptr;
    colind2 = nullptr;
    values2 = nullptr;
    throw "Erreur | CarmaSparseHostObj<T_data>::resize2row_major | erreur lors de la conversion.";
    // exit(EXIT_FAILURE);
  }

  delete[] rowind;
  rowind = rowind2;
  delete[] colind;
  colind = colind2;
  delete[] h_data;
  h_data = values2;

  major_dim = 'R';
}
template <class T_data>
void CarmaSparseHostObj<T_data>::resize2col_major() {
  int32_t i;
  int32_t indice;
  std::vector<std::vector<std::pair<int32_t, int32_t> > > position(
      dims_data[2], std::vector<std::pair<int32_t, int32_t> >());
  int32_t *rowind2 = new int32_t[nz_elem];
  int32_t *colind2 = new int32_t[nz_elem];
  T_data *values2 = new T_data[nz_elem];

  for (i = 0; i < nz_elem; i++) {
    position[colind[i]].push_back(std::make_pair(rowind[i], i));
  }

  indice = 0;
  for (i = 0; i < dims_data[2]; i++) {
    if (position[i].size() > 0) {
      sort(position[i].begin(), position[i].end());
      for (size_t j = 0; j < position[i].size(); j++) {
        colind2[indice] = colind[(position[i][j]).second];
        rowind2[indice] = rowind[(position[i][j]).second];
        values2[indice] = h_data[(position[i][j]).second];
        indice++;
      }
    }
  }
  if (indice != nz_elem) {
    std::cerr << "Erreur | CarmaSparseHostObj<T_data>::resize2col_major | "
                 "erreur lors de la conversion."
              << std::endl;
    delete[] rowind2;
    delete[] colind2;
    delete[] values2;
    rowind2 = nullptr;
    colind2 = nullptr;
    values2 = nullptr;
    throw "Erreur | CarmaSparseHostObj<T_data>::resize2col_major | erreur lors de la conversion.";
    // exit(EXIT_FAILURE);
  }

  delete[] rowind;
  rowind = rowind2;
  delete[] colind;
  colind = colind2;
  delete[] h_data;
  h_data = values2;

  major_dim = 'C';
}

#define EXPLICITE_TEMPLATE(T_data)                                          \
  template CarmaSparseHostObj<T_data>::CarmaSparseHostObj();          \
  template CarmaSparseHostObj<T_data>::~CarmaSparseHostObj();         \
  template CarmaSparseHostObj<T_data>::CarmaSparseHostObj(            \
      CarmaSparseObj<T_data> &M);                                         \
  template CarmaSparseHostObj<T_data>::CarmaSparseHostObj(            \
      CarmaSparseHostObj<T_data> &M);                                    \
  template CarmaSparseHostObj<T_data>::CarmaSparseHostObj(            \
      const int64_t *dims, T_data *M, char order);                             \
  template void CarmaSparseHostObj<T_data>::resize(int32_t nnz_, int32_t dim1_,  \
                                                      int32_t dim2_);           \
  template void CarmaSparseHostObj<T_data>::operator=(                   \
      CarmaSparseHostObj<T_data> &M);                                    \
  template void CarmaSparseHostObj<T_data>::operator=(                   \
      CarmaSparseObj<T_data> &M);                                         \
  template void CarmaSparseHostObj<T_data>::init_from_matrix(            \
      const int64_t *dims, T_data *M, char major_dim = 'R');                    \
  template void CarmaSparseHostObj<T_data>::copy_into_matrix(            \
      T_data *M, char major_dim = 'R');                                      \
  template void CarmaSparseHostObj<T_data>::_create(int32_t nnz_, int32_t dim1_, \
                                                       int32_t dim2_);          \
  template void CarmaSparseHostObj<T_data>::_clear();                    \
  template void carma_gemv(                                                 \
      T_data alpha, CarmaSparseHostObj<T_data> *A,                       \
      CarmaHostObj<T_data> *x, T_data betta, CarmaHostObj<T_data> *y,   \
      void (*ptr_coomv)(char *transa, int64_t *m, int64_t *k, T_data *alpha,      \
                        char *matdescra, T_data *val, int32_t *rowind,          \
                        int32_t *colind, int32_t *nnz, T_data *x, T_data *beta,     \
                        T_data *y));                                        \
  template void carma_gemm(                                                 \
      char op_A, T_data alpha, CarmaSparseHostObj<T_data> *A,            \
      CarmaHostObj<T_data> *B, T_data betta, CarmaHostObj<T_data> *C);  \
  template void CarmaSparseHostObj<T_data>::resize2row_major();           \
  template void CarmaSparseHostObj<T_data>::resize2col_major();

EXPLICITE_TEMPLATE(double)
EXPLICITE_TEMPLATE(float)

#undef EXPLICITE_TEMPLATE
