/*
 * carmasparsehostobj.cpp
 *
 *  Created on: Apr 10, 2014
 *      Author: ???
 */
#include "carma_sparse_host_obj.h"
#include "carma_sparse_obj.h"
#include <algorithm>

template<class T_data>
carma_sparse_host_obj<T_data>::carma_sparse_host_obj() {
  _create(0, 0, 0);
}

template<class T_data>
carma_sparse_host_obj<T_data>::~carma_sparse_host_obj() {
  _clear();
}

template<class T_data>
carma_sparse_host_obj<T_data>::carma_sparse_host_obj(
    carma_sparse_host_obj<T_data>& M) {
  _create(0, 0, 0);
  this->operator=(M);
}

template<class T_data>
carma_sparse_host_obj<T_data>::carma_sparse_host_obj(carma_sparse_obj<T_data>& M) {

  _create(M.nz_elem, M.getDims(1), M.getDims(2));

  cudaMemcpy(h_data, M.d_data, nz_elem * sizeof(T_data),
      cudaMemcpyDeviceToHost);
  cudaMemcpy(rowind, M.d_rowind, (dims_data[1]+1) * sizeof(int),
      cudaMemcpyDeviceToHost);
  cudaMemcpy(colind, M.d_colind, nz_elem * sizeof(int),
      cudaMemcpyDeviceToHost);

  majorDim = M.get_majorDim();
}

template<class T_data>
carma_sparse_host_obj<T_data>::carma_sparse_host_obj(const long *dims, T_data * M, char order){
  _create(0, 0, 0);
  init_from_matrix(dims, M, order);
}

template<class T_data>
void carma_sparse_host_obj<T_data>::resize(int nnz_, int dim1_, int dim2_) {
  if (nz_elem != nnz_) {
    _clear();
    _create(nnz_, dim1_, dim2_);
  } else {
    dims_data[0] = 2;
    dims_data[1] = dim1_;
    dims_data[2] = dim2_;
    majorDim = 'U';
  }
}
//
template<class T_data>
void carma_sparse_host_obj<T_data>::operator=(
    carma_sparse_host_obj<T_data>& M) {
  resize(M.nz_elem, M.getDims(1), M.getDims(2));
  memcpy(h_data, M.h_data, sizeof(T_data) * nz_elem);
  memcpy(rowind, M.rowind, sizeof(int) * (dims_data[1]+1));
  memcpy(colind, M.colind, sizeof(int) * nz_elem);

  majorDim = M.majorDim;

}
//
template<class T_data>
void carma_sparse_host_obj<T_data>::operator=( carma_sparse_obj<T_data> &M) {
  resize(M.nz_elem, M.getDims(1), M.getDims(2));
  cudaMemcpy(h_data, M.d_data, nz_elem*sizeof(T_data), cudaMemcpyDeviceToHost);
  cudaMemcpy(rowind, M.d_rowind, (dims_data[1]+1)*sizeof(int), cudaMemcpyDeviceToHost);
  cudaMemcpy(colind, M.d_colind, nz_elem*sizeof(int), cudaMemcpyDeviceToHost);

  majorDim = M.majorDim;
}

template<class T_data>
void carma_sparse_host_obj<T_data>::init_from_matrix(
    const long *dims, T_data * M, char majorDim) {

  long nb_el = dims[1] * dims[2];
  T_data* valB = M;
  long cmpt = 0;
  for (long i = 0; i < nb_el; i++) {
    if (*valB != 0) {
      cmpt++;
    }
    valB++;
  }

  resize(cmpt, dims[1], dims[2]);
  int *rowcoo = new int[cmpt];

  int *dim1, *dim2, ld;
  if(majorDim=='R'){
    dim1=rowcoo;
    dim2=this->colind;
    ld=dims_data[1];
  } else {
    dim1=this->colind;
    dim2=rowcoo;
    ld=dims_data[2];
  }
  valB = M;
  long index=0;
  for(long i = 0;(index<cmpt)||(i < nb_el); i++) {
    if (*valB != 0) {
      dim1[index] = i / ld;
      dim2[index] = i - dim1[index] * ld;
      h_data[index++]=*valB;
    }
    valB++;
  }

  if(majorDim=='C'){
    //sort rows
    vector<pair<int, pair<int, T_data> > > sM(nz_elem);
    //sM[i].first         -> rowind
    //sM[i].second.first  -> colind
    //sM[i].second.second -> value
    for (int i = 0; i < nz_elem; i++) {
      sM[i].first = rowcoo[i];
      sM[i].second.first = colind[i];
      sM[i].second.second = h_data[i];
    }
    sort(sM.begin(), sM.end());
    for (int i = 0; i < nz_elem; i++) {
      rowcoo[i] = sM[i].first;
      colind[i] = sM[i].second.first;
      h_data[i] = sM[i].second.second;
    }
  }

  //coo2csr
  rowind[0]=0;
  long csr_index=0;
  index=0;
  while(csr_index<ld+1){
    while(rowcoo[index]==csr_index){
      index++;
    }
    rowind[++csr_index]=index;
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

  this->majorDim = majorDim;
}

template<class T_data>
void carma_sparse_host_obj<T_data>::copy_into_matrix(
    T_data * M, char majorDim) {
  int ld;
  if(majorDim=='R'){
    ld=dims_data[1];
  } else {
    ld=dims_data[2];
  }
  T_data* val = this->h_data;
  int index=0;
  for(int i=0; i<ld; i++){
    while(index<rowind[i+1]){
      if(majorDim=='R'){
        M[i*rowind[i]+colind[index]]=*val;
      } else {
        M[i+ld*colind[index]]=*val;
      }

      //DEBUG_TRACE("%d %d %d %d\n",i,index, rowind[i+1], rowind[i]+ld*colind[index]);
      index++;val++;
    }
  }
}

//
template<class T_data>
void carma_sparse_host_obj<T_data>::_create(int nnz_, int dim1_, int dim2_) {
  nz_elem = nnz_;
  dims_data[0] = 2;
  dims_data[1] = dim1_;
  dims_data[2] = dim2_;
  h_data = new T_data[nz_elem];
  rowind = new int[dim1_+1];
  colind = new int[nz_elem];
  majorDim = 'U';
}

//
template<class T_data>
void carma_sparse_host_obj<T_data>::_clear() {
  if (h_data == NULL || rowind == NULL || colind == NULL) {
    cerr << "Error | carma_sparse_host_obj<T_data>::_clear | double clear"
        << endl;
    throw "Error | carma_sparse_host_obj<T_data>::_clear | double clear";
    //exit(EXIT_FAILURE);
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
  majorDim = 'U';
}
//
template<class T_data>
void carma_gemv(T_data alpha, carma_sparse_host_obj<T_data>* A,
    carma_host_obj<T_data>* x, T_data betta, carma_host_obj<T_data>* y,
    void (*ptr_coomv)(char *transa, long *m, long *k, T_data *alpha,
        char *matdescra, T_data *val, int *rowind, int *colind, int *nnz,
        T_data *x, T_data *beta, T_data *y)) {
  if (A->getDims(2) != x->getDims(1) || A->getDims(1) != y->getDims(1)) {
    cerr << "Error | kp_cscmv | diminsion problem" << endl;
    throw "Error | kp_cscmv | diminsion problem";
    //exit(EXIT_FAILURE);
  }
//   A.check();
  long dim1 = A->getDims(1);
  long dim2 = A->getDims(2);
  int nz_elem = A->nz_elem;

  char transa[]="N";
  char matdescra[]="GFFFFFFFF";
  ptr_coomv(transa, &dim1, &dim2, &alpha, matdescra, A->getData(), A->rowind,
      A->colind, &nz_elem, x->getData(), &betta, y->getData());
//   mkl_dcscmv("N", &A.dim1, &A.dim2, &alpha, "GFFFFFFF",
//        A.values, A.rows, A.pointerB, A.pointerB + 1,
//        x.d,  &betta, y.d);

}

template<class T_data>
void carma_gemm(char op_A, T_data alpha, carma_sparse_host_obj<T_data>* A,
    carma_host_obj<T_data>* B, T_data betta, carma_host_obj<T_data>* C) {
//   A.check();
  /* FIXME: à coder sans MKL ou pas...
   long dimA1=A->getDims(1);
   long dimA2=A->getDims(2);
   long dimB1=B->getDims(1);
   long dimB2=B->getDims(2);
   long dimC1=C->getDims(1);
   long dimC2=C->getDims(2);
   long nz_elem=A->nz_elem;

   mkl_dcoomm(&op_A, &dimA1, &dimC2, &dimA2, &alpha, "GFFFFFFF", A->getData(),
   A->rowind, A->colind, &nz_elem, B->getData(), &dimB1, &betta, C->getData(), &dimC1);
   */

  //   mkl_dcscmm("N", &A.dim1, &C.dim2,  &A.dim2, &alpha, "GCCCCCCC",
//        A.values, A.rows, A.pointerB, A.pointerB+1,
//        B.d, &B.tda,  &betta, C.d, & C.tda);
}

template<class T_data>
void carma_sparse_host_obj<T_data>::resize2rowMajor() {
  int i;
  int indice;
  vector<vector<pair<int, int> > > position(dims_data[1],
      vector<pair<int, int> >());
  int* rowind2 = new int[nz_elem];
  int* colind2 = new int[nz_elem];
  T_data* values2 = new T_data[nz_elem];

  //cout<<"dim1="<<this->dim1<<" dim2="<<this->dim2<<" nnz="<<this->nnz<<endl;
  //cout<<"i="<<172248<<" rowind[i]="<<rowind[172248]<<" colind[i]="<<colind[172248]<<endl;

  for (i = 0; i < nz_elem; i++) {
    //cout<<"i="<<i<<" rowind[i]="<<rowind[i]-1<<" colind[i]="<<colind[i]-1<<endl;
    position[rowind[i]].push_back(make_pair(colind[i], i));

  }

  indice = 0;
  for (i = 0; i < dims_data[1]; i++) {
    if (position[i].size() > 0) {
      sort(position[i].begin(), position[i].end());
      for ( size_t j = 0; j < position[i].size(); j++) {
        rowind2[indice] = rowind[(position[i][j]).second];
        colind2[indice] = colind[(position[i][j]).second];
        values2[indice] = h_data[(position[i][j]).second];
        indice++;
      }
    }
  }
  if (indice != nz_elem) {
    cerr
        << "Erreur | carma_sparse_host_obj<T_data>::resize2rowMajor | erreur lors de la conversion."
        << endl;
    throw "Erreur | carma_sparse_host_obj<T_data>::resize2rowMajor | erreur lors de la conversion.";
    //exit(EXIT_FAILURE);
  }

  delete[] rowind;
  rowind = rowind2;
  delete[] colind;
  colind = colind2;
  delete[] h_data;
  h_data = values2;

  majorDim = 'R';

}
template<class T_data>
void carma_sparse_host_obj<T_data>::resize2colMajor() {
  int i;
  int indice;
  vector<vector<pair<int, int> > > position(dims_data[2],
      vector<pair<int, int> >());
  int* rowind2 = new int[nz_elem];
  int* colind2 = new int[nz_elem];
  T_data* values2 = new T_data[nz_elem];

  for (i = 0; i < nz_elem; i++) {
    position[colind[i]].push_back(make_pair(rowind[i], i));
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
    cerr
        << "Erreur | carma_sparse_host_obj<T_data>::resize2colMajor | erreur lors de la conversion."
        << endl;
    throw "Erreur | carma_sparse_host_obj<T_data>::resize2colMajor | erreur lors de la conversion.";
    //exit(EXIT_FAILURE);
  }

  delete[] rowind;
  rowind = rowind2;
  delete[] colind;
  colind = colind2;
  delete[] h_data;
  h_data = values2;

  majorDim = 'C';

}

#define EXPLICITE_TEMPLATE(T_data) template carma_sparse_host_obj<T_data>::carma_sparse_host_obj(); \
    template carma_sparse_host_obj<T_data>::~carma_sparse_host_obj(); \
    template carma_sparse_host_obj<T_data>::carma_sparse_host_obj( \
        carma_sparse_obj<T_data>& M); \
        template carma_sparse_host_obj<T_data>::carma_sparse_host_obj( \
            carma_sparse_host_obj<T_data>& M); \
    template carma_sparse_host_obj<T_data>::carma_sparse_host_obj( \
        const long *dims, T_data * M, char order); \
    template void carma_sparse_host_obj<T_data>::resize(int nnz_, int dim1_, int dim2_); \
    template void carma_sparse_host_obj<T_data>::operator=( carma_sparse_host_obj<T_data>& M); \
    template void carma_sparse_host_obj<T_data>::operator=( carma_sparse_obj<T_data>& M); \
    template void carma_sparse_host_obj<T_data>::init_from_matrix(const long *dims, T_data * M, char majorDim='R'); \
    template void carma_sparse_host_obj<T_data>::copy_into_matrix(T_data * M, char majorDim='R'); \
    template void carma_sparse_host_obj<T_data>::_create(int nnz_, int dim1_, int dim2_); \
    template void carma_sparse_host_obj<T_data>::_clear(); \
    template void carma_gemv(T_data alpha, carma_sparse_host_obj<T_data>* A, \
        carma_host_obj<T_data>* x, T_data betta, carma_host_obj<T_data>* y, \
        void (*ptr_coomv)(char *transa, long *m, long *k, T_data *alpha, \
            char *matdescra, T_data *val, int *rowind, int *colind, \
            int *nnz, T_data *x, T_data *beta, T_data *y)); \
    template void carma_gemm(char op_A, T_data alpha, carma_sparse_host_obj<T_data>* A, \
        carma_host_obj<T_data>* B, T_data betta, carma_host_obj<T_data>* C); \
    template void carma_sparse_host_obj<T_data>::resize2rowMajor(); \
    template void carma_sparse_host_obj<T_data>::resize2colMajor();

EXPLICITE_TEMPLATE(double)
EXPLICITE_TEMPLATE(float)


#undef EXPLICITE_TEMPLATE
