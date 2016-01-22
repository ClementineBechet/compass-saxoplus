from naga cimport *

import numpy as np
cimport numpy as np

include "../par.pxi"

from libc.stdlib cimport malloc, free
from libcpp.cast cimport dynamic_cast
from libcpp.string cimport string
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.pair cimport pair

from libc.stdint cimport uintptr_t

from cpython.string cimport PyString_AsString

IF USE_MPI == 1:
    print "using mpi"
    #from mpi4py import MPI
    from mpi4py cimport MPI
    #cimport mpi4py.MPI as MPI
    # C-level cdef, typed, Python objects
    from mpi4py cimport mpi_c as mpi
    #from mpi4py cimport libmpi as mpi
    #cimport mpi4py.libmpi as mpi


from libc.math cimport sin

cdef np.float32_t dtor = (np.pi/180.)

from shesha_telescope import *
from shesha_telescope cimport *
from shesha_param import *
from shesha_param cimport *
from shesha_sensors cimport *
from shesha_atmos cimport *
from shesha_dms cimport *

#################################################
# P-Class Target
#################################################
cdef class Target:
    cdef sutra_target *target
    """sutra_target object"""
    cdef readonly int ntargets
    """number of targets"""
    cdef readonly int apod
    """boolean for apodizer"""
    cdef readonly np.ndarray Lambda
    """observation wavelength for each target"""
    cdef readonly np.ndarray xpos
    """x positions on sky (in arcsec) for each target"""
    cdef readonly np.ndarray ypos
    """y positions on sky (in arcsec) for each target"""
    cdef readonly np.ndarray mag
    """magnitude for each target"""
    cdef int device
    """device number"""
    cdef naga_context context

    cpdef dmtrace(self,int ntar, Dms dms, int reset=?)

IF USE_BRAMA == 1:
    cdef extern from *:
        sutra_target_brama* dynamic_cast_target_brama_ptr "dynamic_cast<sutra_target_brama*>" (sutra_target*) except NULL
    
    #################################################
    # P-Class Target_brama
    #################################################
    cdef class Target_brama(Target):
        cpdef publish(self)
