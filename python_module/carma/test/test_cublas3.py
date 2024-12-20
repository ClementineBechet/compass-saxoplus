#
# This file is part of COMPASS <https://github.com/COSMIC-RTC/compass>
#
# COMPASS is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# COMPASS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with COMPASS. If not, see <https://www.gnu.org/licenses/>.
#
# Copyright (C) 2011-2024 COSMIC Team
import numpy as np
import carma as ch
import numpy.testing as npt
import time

dec = 4
prec = 10**-dec

sizem = 128
sizen = 256
sizek = 512

seed = np.int32(time.perf_counter())

print("")
print("Test cublas 3")
print("precision: ", prec)

c = ch.context.get_instance()


def test_float_gemm():
    # function gemm
    # testing: C=A.B+C
    # A,B,C matrices

    # generating random matrices A,B,C and associated CarmaObj

    # np.random.seed(seed)
    # A = A.dot(A.T)
    # B = B.dot(B.T)

    matA = ch.obj_float(c, np.random.randn(sizem, sizek))
    matAT = ch.obj_float(c, np.random.randn(sizek, sizem))
    matB = ch.obj_float(c, np.random.randn(sizek, sizen))
    matBT = ch.obj_float(c, np.random.randn(sizen, sizek))
    matC = ch.obj_float(c, np.random.randn(sizem, sizen))
    matC2 = ch.obj_float(c, np.random.randn(sizem, sizen))
    matC3 = ch.obj_float(c, np.random.randn(sizem, sizen))

    # matA.random_host(seed, 'U')
    # matAT.random_host(seed, 'U')
    # matB.random_host(seed + 2, 'U')
    # matBT.random_host(seed + 2, 'U')
    # matC.random_host(seed + 3, 'U')
    # matC2.random_host(seed + 3, 'U')
    # matC3.random_host(seed + 3, 'U')

    A = np.array(matA)
    AT = np.array(matAT)
    B = np.array(matB)
    BT = np.array(matBT)
    C = np.array(matC)
    C2 = np.array(matC2)
    C3 = np.array(matC3)

    alpha = 2
    beta = 1

    # matrices product
    matA.gemm(matB, "n", "n", alpha, matC, beta)
    matAT.gemm(matB, "t", "n", alpha, matC2, beta)
    matAT.gemm(matBT, "t", "t", alpha, matC3, beta)
    matC4 = matA.gemm(matB, "n", "n", alpha)
    matC5 = matAT.gemm(matB, "t", "n", alpha)
    matC6 = matAT.gemm(matBT, "t", "t", alpha)

    C = alpha * A.dot(B) + beta * C
    C2 = alpha * AT.T.dot(B) + beta * C2
    C3 = alpha * AT.T.dot(BT.T) + beta * C3
    C4 = alpha * A.dot(B)
    C5 = alpha * AT.T.dot(B)
    C6 = alpha * AT.T.dot(BT.T)

    npt.assert_array_almost_equal(C, np.array(matC), decimal=dec - 1)
    npt.assert_array_almost_equal(C2, np.array(matC2), decimal=dec - 1)
    npt.assert_array_almost_equal(C3, np.array(matC3), decimal=dec - 1)
    npt.assert_array_almost_equal(C4, np.array(matC4), decimal=dec - 1)
    npt.assert_array_almost_equal(C5, np.array(matC5), decimal=dec - 1)
    npt.assert_array_almost_equal(C6, np.array(matC6), decimal=dec - 1)


def test_float_symm():
    # function symm
    # testing: C=A.B+C
    # A ssymetric matrix, B,C matrices
    A = np.random.randn(sizek, sizek)
    B = np.random.randn(sizek, sizen)
    C = np.random.randn(sizek, sizen)

    # generating random matrices and associated CarmaObj
    matA = ch.obj_float(c, A)  # np.random.randn(sizek, sizek)))
    matB = ch.obj_float(c, B)  # np.random.randn(sizek, sizen)))
    matC = ch.obj_float(c, C)  # np.random.randn(sizek, sizen)))

    # matA.random_host(seed, 'U')
    # matB.random_host(seed + 2, 'U')
    # matC.random_host(seed + 3, 'U')

    # A symetric
    A = np.array(matA)
    A = (A + A.T) / 2
    matA.host2device(A)
    B = np.array(matB)
    C = np.array(matC)

    # matrices multiplication
    t1 = time.perf_counter()
    matA.symm(matB, 1, matC, 1)
    t2 = time.perf_counter()
    C = A.dot(B) + C
    t3 = time.perf_counter()

    matC2 = matA.symm(matB)
    C2 = A.dot(B)

    print("")
    print("test symm:")
    print("execution time (s)")
    print("python: ", t3 - t2)
    print("carma : ", t2 - t1)

    # test results
    res = np.array(matC)

    npt.assert_almost_equal(C, res, decimal=dec - 1)
    npt.assert_almost_equal(C2, np.array(matC2), decimal=dec - 1)

    # M = np.argmax(np.abs(res - C))
    # d = 5
    # if (0 < np.abs(C.item(M))):


# d = 10**np.ceil(np.log10(np.abs(C.item(M))))

# print(res.item(M))
# print(C.item(M))

# npt.assert_almost_equal(C.item(M) / d, res.item(M) / d, decimal=dec)


def test_float_dgmm():
    # function dgmm
    # testing: C=A.d
    # C,A matrices, d vector (diagonal matrix as a vector)

    # generating random matrices and associated CarmaObj
    matA = ch.obj_float(c, np.random.randn(sizek, sizek))
    Vectx = ch.obj_float(c, np.random.randn(sizek))

    # matA.random_host(seed, 'U')
    # Vectx.random_host(seed + 2, 'U')
    A = np.array(matA)
    x = np.array(Vectx)

    # matrices product
    t1 = time.perf_counter()
    matC = matA.dgmm(Vectx)
    t2 = time.perf_counter()
    C = A * x
    t3 = time.perf_counter()

    print("")
    print("test dgmm:")
    print("execution time (s)")
    print("python: ", t3 - t2)
    print("carma : ", t2 - t1)

    # test results
    res = np.array(matC)

    npt.assert_almost_equal(C, res, decimal=dec)

    # M = np.argmax(np.abs(res - C))
    # d = 5
    # if (0 < np.abs(C.item(M))):
    #     d = 10**np.ceil(np.log10(np.abs(C.item(M))))

    # print(res.item(M))
    # print(C.item(M))

    # npt.assert_almost_equal(C.item(M) / d, res.item(M) / d, decimal=dec)


def test_float_syrk():
    # function syrk
    # testing: C=A.transpose(A)+C
    # A matrix, C symetric matrix

    # generating random matrices and associated CarmaObj
    matA = ch.obj_float(c, np.random.randn(sizen, sizek))
    matC = ch.obj_float(c, np.random.randn(sizen, sizen))

    # matA.random_host(seed, 'U')
    # matC.random_host(seed + 2, 'U')

    A = np.array(matA)
    C = np.array(matC)
    # syrk: C matrix is symetric
    C = (C + C.T) / 2

    matC.host2device(C)

    # matrices product
    t1 = time.perf_counter()
    matA.syrk(matC=matC, beta=1)
    t2 = time.perf_counter()
    C = A.dot(A.T) + C
    t3 = time.perf_counter()

    print("")
    print("test syrk:")
    print("execution time (s)")
    print("python: ", t3 - t2)
    print("carma : ", t2 - t1)

    # test results
    res = np.array(matC)

    # only upper triangle is computed
    C = np.triu(C).flatten()
    res = np.triu(res).flatten()
    npt.assert_almost_equal(C, res, decimal=dec - 1)

    # M = np.argmax(np.abs(res - C))
    # d = 5
    # if (0 < np.abs(C.item(M))):
    #     d = 10**np.ceil(np.log10(np.abs(C.item(M))))

    # print(res.item(M))
    # print(C.item(M))

    # npt.assert_almost_equal(C.item(M) / d, res.item(M) / d, decimal=dec)


def test_float_syrkx():
    # function syrkx
    # testing: C=A.transpose(B)+C
    # A matrix, C symetric matrix

    # generating random matrices and associated CarmaObj
    matA = ch.obj_float(c, np.random.randn(sizen, sizek))
    matB = ch.obj_float(c, np.random.randn(sizen, sizek))
    matC = ch.obj_float(c, np.random.randn(sizen, sizen))

    # matA.random_host(seed, 'U')
    # matB.random_host(seed + 2, 'U')
    # matC.random_host(seed + 3, 'U')

    A = np.array(matA)
    B = np.array(matB)
    C = np.array(matC)

    # C is symetric
    C = np.dot(C, C.T)

    matC.host2device(C)

    # matrices product
    t1 = time.perf_counter()
    matA.syrkx(matB, alpha=1, matC=matC, beta=1)
    t2 = time.perf_counter()
    C = A.dot(B.T) + C.T
    t3 = time.perf_counter()

    print("")
    print("test syrkx:")
    print("execution time (s)")
    print("python: ", t3 - t2)
    print("carma : ", t2 - t1)

    # test results
    res = np.array(matC)

    # only upper triangle is computed
    res = np.triu(res)
    C = np.triu(C)
    npt.assert_almost_equal(C, res, decimal=dec - 1)

    # M = np.argmax(np.abs(res - C))
    # d = 5
    # if (0 < np.abs(C.item(M))):
    #     d = 10**np.ceil(np.log10(np.abs(C.item(M))))

    # print(res.item(M))
    # print(C.item(M))

    # npt.assert_almost_equal(C.item(M) / d, res.item(M) / d, decimal=dec)


def test_float_geam():
    # function geam
    # testing: C=A.B
    # A,B matrices

    # generating random matrices and associated CarmaObj
    matA = ch.obj_float(c, np.random.randn(sizem, sizen))
    matB = ch.obj_float(c, np.random.randn(sizem, sizen))

    # matA.random_host(seed, 'U')
    # matB.random_host(seed + 2, 'U')

    A = np.array(matA)
    B = np.array(matB)

    # matrices product
    t1 = time.perf_counter()
    C = A + B
    t2 = time.perf_counter()
    matC = matA.geam(matB, beta=1)
    t3 = time.perf_counter()

    print("")
    print("test geam:")
    print("execution time (s)")
    print("python: ", t3 - t2)
    print("carma : ", t2 - t1)

    # testing result

    npt.assert_array_almost_equal(C, np.array(matC), decimal=dec)


def test_double_gemm():
    # function gemm
    # testing: C=A.B+C
    # A,B,C matrices

    # generating random matrices A,B,C and associated CarmaObj

    # np.random.seed(seed)
    # A = A.dot(A.T)
    # B = B.dot(B.T)

    matA = ch.obj_double(c, np.random.randn(sizem, sizek))
    matAT = ch.obj_double(c, np.random.randn(sizek, sizem))
    matB = ch.obj_double(c, np.random.randn(sizek, sizen))
    matBT = ch.obj_double(c, np.random.randn(sizen, sizek))
    matC = ch.obj_double(c, np.random.randn(sizem, sizen))
    matC2 = ch.obj_double(c, np.random.randn(sizem, sizen))
    matC3 = ch.obj_double(c, np.random.randn(sizem, sizen))

    # matA.random_host(seed, 'U')
    # matAT.random_host(seed, 'U')
    # matB.random_host(seed + 2, 'U')
    # matBT.random_host(seed + 2, 'U')
    # matC.random_host(seed + 3, 'U')
    # matC2.random_host(seed + 3, 'U')
    # matC3.random_host(seed + 3, 'U')

    A = np.array(matA)
    AT = np.array(matAT)
    B = np.array(matB)
    BT = np.array(matBT)
    C = np.array(matC)
    C2 = np.array(matC2)
    C3 = np.array(matC3)

    alpha = 2
    beta = 1

    # matrices product
    matA.gemm(matB, "n", "n", alpha, matC, beta)
    matAT.gemm(matB, "t", "n", alpha, matC2, beta)
    matAT.gemm(matBT, "t", "t", alpha, matC3, beta)
    matC4 = matA.gemm(matB, "n", "n", alpha)
    matC5 = matAT.gemm(matB, "t", "n", alpha)
    matC6 = matAT.gemm(matBT, "t", "t", alpha)

    C = alpha * A.dot(B) + beta * C
    C2 = alpha * AT.T.dot(B) + beta * C2
    C3 = alpha * AT.T.dot(BT.T) + beta * C3
    C4 = alpha * A.dot(B)
    C5 = alpha * AT.T.dot(B)
    C6 = alpha * AT.T.dot(BT.T)

    npt.assert_array_almost_equal(C, np.array(matC), decimal=2 * dec - 1)
    npt.assert_array_almost_equal(C2, np.array(matC2), decimal=2 * dec - 1)
    npt.assert_array_almost_equal(C3, np.array(matC3), decimal=2 * dec - 1)
    npt.assert_array_almost_equal(C4, np.array(matC4), decimal=2 * dec - 1)
    npt.assert_array_almost_equal(C5, np.array(matC5), decimal=2 * dec - 1)
    npt.assert_array_almost_equal(C6, np.array(matC6), decimal=2 * dec - 1)


def test_double_symm():
    # function symm
    # testing: C=A.B+C
    # A ssymetric matrix, B,C matrices
    A = np.random.randn(sizek, sizek)
    B = np.random.randn(sizek, sizen)
    C = np.random.randn(sizek, sizen)

    # generating random matrices and associated CarmaObj
    matA = ch.obj_double(c, A)  # np.random.randn(sizek, sizek)))
    matB = ch.obj_double(c, B)  # np.random.randn(sizek, sizen)))
    matC = ch.obj_double(c, C)  # np.random.randn(sizek, sizen)))

    # matA.random_host(seed, 'U')
    # matB.random_host(seed + 2, 'U')
    # matC.random_host(seed + 3, 'U')

    # A symetric
    A = np.array(matA)
    A = (A + A.T) / 2
    matA.host2device(A)
    B = np.array(matB)
    C = np.array(matC)

    # matrices multiplication
    t1 = time.perf_counter()
    matA.symm(matB, 1, matC, 1)
    t2 = time.perf_counter()
    C = A.dot(B) + C
    t3 = time.perf_counter()

    matC2 = matA.symm(matB)
    C2 = A.dot(B)

    print("")
    print("test symm:")
    print("execution time (s)")
    print("python: ", t3 - t2)
    print("carma : ", t2 - t1)

    # test results
    res = np.array(matC)

    npt.assert_almost_equal(C, res, decimal=2 * dec - 1)
    npt.assert_almost_equal(C2, np.array(matC2), decimal=2 * dec - 1)

    # M = np.argmax(np.abs(res - C))
    # d = 5
    # if (0 < np.abs(C.item(M))):


# d = 10**np.ceil(np.log10(np.abs(C.item(M))))

# print(res.item(M))
# print(C.item(M))

# npt.assert_almost_equal(C.item(M) / d, res.item(M) / d, decimal=2*dec)


def test_double_dgmm():
    # function dgmm
    # testing: C=A.d
    # C,A matrices, d vector (diagonal matrix as a vector)

    # generating random matrices and associated CarmaObj
    matA = ch.obj_double(c, np.random.randn(sizek, sizek))
    Vectx = ch.obj_double(c, np.random.randn(sizek))

    # matA.random_host(seed, 'U')
    # Vectx.random_host(seed + 2, 'U')
    A = np.array(matA)
    x = np.array(Vectx)

    # matrices product
    t1 = time.perf_counter()
    matC = matA.dgmm(Vectx)
    t2 = time.perf_counter()
    C = A * x
    t3 = time.perf_counter()

    print("")
    print("test dgmm:")
    print("execution time (s)")
    print("python: ", t3 - t2)
    print("carma : ", t2 - t1)

    # test results
    res = np.array(matC)

    npt.assert_almost_equal(C, res, decimal=2 * dec)

    # M = np.argmax(np.abs(res - C))
    # d = 5
    # if (0 < np.abs(C.item(M))):
    #     d = 10**np.ceil(np.log10(np.abs(C.item(M))))

    # print(res.item(M))
    # print(C.item(M))

    # npt.assert_almost_equal(C.item(M) / d, res.item(M) / d, decimal=2*dec)


def test_double_syrk():
    # function syrk
    # testing: C=A.transpose(A)+C
    # A matrix, C symetric matrix

    # generating random matrices and associated CarmaObj
    matA = ch.obj_double(c, np.random.randn(sizen, sizek))
    matC = ch.obj_double(c, np.random.randn(sizen, sizen))

    # matA.random_host(seed, 'U')
    # matC.random_host(seed + 2, 'U')

    A = np.array(matA)
    C = np.array(matC)
    # syrk: C matrix is symetric
    C = (C + C.T) / 2

    matC.host2device(C)

    # matrices product
    t1 = time.perf_counter()
    matA.syrk(matC=matC, beta=1)
    t2 = time.perf_counter()
    C = A.dot(A.T) + C
    t3 = time.perf_counter()

    print("")
    print("test syrk:")
    print("execution time (s)")
    print("python: ", t3 - t2)
    print("carma : ", t2 - t1)

    # test results
    res = np.array(matC)

    # only upper triangle is computed
    C = np.triu(C).flatten()
    res = np.triu(res).flatten()
    npt.assert_almost_equal(C, res, decimal=2 * dec - 1)

    # M = np.argmax(np.abs(res - C))
    # d = 5
    # if (0 < np.abs(C.item(M))):
    #     d = 10**np.ceil(np.log10(np.abs(C.item(M))))

    # print(res.item(M))
    # print(C.item(M))

    # npt.assert_almost_equal(C.item(M) / d, res.item(M) / d, decimal=2*dec)


def test_double_syrkx():
    # function syrkx
    # testing: C=A.transpose(B)+C
    # A matrix, C symetric matrix

    # generating random matrices and associated CarmaObj
    matA = ch.obj_double(c, np.random.randn(sizen, sizek))
    matB = ch.obj_double(c, np.random.randn(sizen, sizek))
    matC = ch.obj_double(c, np.random.randn(sizen, sizen))

    # matA.random_host(seed, 'U')
    # matB.random_host(seed + 2, 'U')
    # matC.random_host(seed + 3, 'U')

    A = np.array(matA)
    B = np.array(matB)
    C = np.array(matC)

    # C is symetric
    C = np.dot(C, C.T)

    matC.host2device(C)

    # matrices product
    t1 = time.perf_counter()
    matA.syrkx(matB, alpha=1, matC=matC, beta=1)
    t2 = time.perf_counter()
    C = A.dot(B.T) + C.T
    t3 = time.perf_counter()

    print("")
    print("test syrkx:")
    print("execution time (s)")
    print("python: ", t3 - t2)
    print("carma : ", t2 - t1)

    # test results
    res = np.array(matC)

    # only upper triangle is computed
    res = np.triu(res)
    C = np.triu(C)
    npt.assert_almost_equal(C, res, decimal=2 * dec - 1)

    # M = np.argmax(np.abs(res - C))
    # d = 5
    # if (0 < np.abs(C.item(M))):
    #     d = 10**np.ceil(np.log10(np.abs(C.item(M))))

    # print(res.item(M))
    # print(C.item(M))

    # npt.assert_almost_equal(C.item(M) / d, res.item(M) / d, decimal=2*dec)


def test_double_geam():
    # function geam
    # testing: C=A.B
    # A,B matrices

    # generating random matrices and associated CarmaObj
    matA = ch.obj_double(c, np.random.randn(sizem, sizen))
    matB = ch.obj_double(c, np.random.randn(sizem, sizen))

    # matA.random_host(seed, 'U')
    # matB.random_host(seed + 2, 'U')

    A = np.array(matA)
    B = np.array(matB)

    # matrices product
    t1 = time.perf_counter()
    C = A + B
    t2 = time.perf_counter()
    matC = matA.geam(matB, beta=1)
    t3 = time.perf_counter()

    print("")
    print("test geam:")
    print("execution time (s)")
    print("python: ", t3 - t2)
    print("carma : ", t2 - t1)

    # testing result

    npt.assert_array_almost_equal(C, np.array(matC), decimal=2 * dec)
