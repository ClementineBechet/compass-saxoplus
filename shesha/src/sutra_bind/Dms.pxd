cimport numpy as np

from libc.stdlib cimport malloc, free
from libcpp.cast cimport dynamic_cast
from libcpp.string cimport string
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.pair cimport pair

from libc.stdint cimport uintptr_t

from cpython.string cimport PyString_AsString

from libc.math cimport sin

from naga_sparse_obj cimport *

# FIXME
from shesha_param cimport *
from shesha_param import *
from shesha_sensors cimport *

#################################################
# P-Class Dms
#################################################
cdef class Dms:
    cdef sutra_dms *dms  # Sutra DMS object
    cdef int device  # GPU number
    cpdef add_dm(self, bytes type_dm, float alt, long dim, long ninflu,
                long influsize, long ninflupos, long npts, float puhs4imat,
                int device=?)
    cpdef remove_dm(self,bytes type_dm, float alt)

    cpdef load_pzt(self, float alt,
                    np.ndarray[ndim=3,dtype=np.float32_t] influ,
                    np.ndarray[ndim=1,dtype=np.int32_t] influpos,
                    np.ndarray[ndim=1,dtype=np.int32_t] npoints,
                    np.ndarray[ndim=1,dtype=np.int32_t] istart,
                    np.ndarray[ndim=1,dtype=np.int32_t] xoff,
                    np.ndarray[ndim=1,dtype=np.int32_t] yoff,
                    np.ndarray[ndim=2,dtype=np.float32_t] kern)

    #TODO dims of arrays
    cpdef load_kl(self,float alt, np.ndarray[ndim=1,dtype=np.float32_t] rabas,
                    np.ndarray[ndim=1,dtype=np.float32_t] azbas,
                    np.ndarray[ndim=1,dtype=np.int32_t] ord,
                    np.ndarray[ndim=1,dtype=np.float32_t] cr,
                    np.ndarray[ndim=1,dtype=np.float32_t] cp)

    cpdef load_tt(self, float alt, np.ndarray[ndim=3, dtype=np.float32_t] influ)

    cpdef set_full_comm(self, np.ndarray[ndim=1, dtype=np.float32_t] comm,
                        bool shape_dm=?)
    cpdef set_comm(self, bytes type_dm, float alt,
                   np.ndarray[ndim=1, dtype=np.float32_t] comm,
                   bool shape_dm=?)
    cpdef shape_dm(self, bytes type_dm, float alt)

    cpdef computeKLbasis(self, bytes type_dm, float alt,
                         np.ndarray[ndim=1, dtype=np.float32_t] xpos, np.ndarray[ndim=1, dtype=np.float32_t] ypos,
                         np.ndarray[ndim=1, dtype=np.int32_t] indx_pup, long dim, float norm, float ampli)
    cpdef get_KLbasis(self, bytes type_dm, float alt)
    cpdef get_IFsparse(self, bytes type_dm, float alt, np.ndarray[ndim=1, dtype=np.int32_t] indx_pup)
    cpdef get_IFtt(self, bytes type_dm, float alt, np.ndarray[ndim=1, dtype=np.int32_t] indx_pup)

    cpdef getComm(self, bytes type_dm, float alt)
    cpdef getInflu(self, bytes type_dm, float alt)
    cpdef comp_oneactu(self, bytes type_dm, float alt, int nactu, float ampli)