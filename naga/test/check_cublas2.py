import numpy as np
import naga as ch
import numpy.testing as npt
import time

dec = 4
prec = 10**-dec

sizem = 512
sizen = 1024

seed = 1234  # int(time.clock()*10**6)

print("")
print("Test cublas 2")
print("Precision: ", prec)

c = ch.naga_context.get_instance()

#generatig random naga_obj 2d

#generating random symetric naga_obj 2d

#generating 3 random naga_obj 1d


def test_gemv():

    #function gemv
    # testing: y=A.x
    # x and y are vector, A a matrix

    A = np.empty((sizem, sizen), dtype=np.float32)
    AT = np.empty((sizen, sizem), dtype=np.float32)
    x = np.empty((sizen), dtype=np.float32)
    y = np.empty((sizem), dtype=np.float32)
    y2 = np.empty((sizem), dtype=np.float32)

    alpha = 2
    beta = 1

    Mat = ch.naga_obj_float(c, A)
    MatT = ch.naga_obj_float(c, AT)
    Mat.random_host(seed, 'U')
    MatT.random_host(seed * 2, 'U')

    Vectx = ch.naga_obj_float(c, x)
    Vecty = ch.naga_obj_float(c, y)
    Vectx.random_host(seed * 3, 'U')
    Vecty.random_host(seed * 4, 'U')

    A = np.array(Mat)
    AT = np.array(MatT)
    x = np.array(Vectx)
    y = np.array(Vecty)

    y = alpha * A.dot(x) + beta * y
    y2 = alpha * A.dot(x)
    y3 = alpha * AT.T.dot(x)

    # Vecty = ch.naga_obj_float(c, np.zeros((sizem), dtype=np.float32))

    Mat.gemv(Vectx, alpha, 'N', Vecty, beta)
    Vecty_2 = Mat.gemv(Vectx, alpha, 'N')
    Vecty_3 = MatT.gemv(Vectx, alpha, 'T')

    npt.assert_array_almost_equal(y, np.array(Vecty), decimal=dec - 1)
    npt.assert_array_almost_equal(y2, np.array(Vecty_2), decimal=dec - 1)
    npt.assert_array_almost_equal(y3, np.array(Vecty_3), decimal=dec - 1)


def test_ger():
    # function ger
    # testing: A= x.y
    #   and  : A= x.y+ A
    # x and y are vectors, A a matrix
    Mat = ch.naga_obj_float(c, np.zeros((sizem, sizen), dtype=np.float32))
    Mat.random_host(seed * 2, 'U')

    Vectx = ch.naga_obj_float(c, np.zeros((sizen), dtype=np.float32))
    Vectx.random_host(seed * 3, 'U')

    Vecty = ch.naga_obj_float(c, np.zeros((sizem), dtype=np.float32))
    Vecty.random_host(seed * 4, 'U')

    x = np.array(Vectx)
    A = np.array(Mat)
    y = np.array(Vecty)

    caOresA = Vecty.ger(Vectx, Mat)
    caOresB = Vecty.ger(Vectx)

    A = np.outer(y, x) + A
    B = np.outer(y, x)

    # npt.assert_array_almost_equal(A, np.array(caOresA), decimal=dec)
    npt.assert_array_almost_equal(B, np.array(caOresB), decimal=dec)


def test_symv():
    #function symv
    # testing: y=A.x
    # x and y are vector, A a symetric matrix

    MatSym = ch.naga_obj_float(c, np.zeros((sizem, sizem), dtype=np.float32))
    MatSym.random_host(seed * 2, 'U')
    data_R = np.array(MatSym)
    data_R = data_R + data_R.T
    MatSym = ch.naga_obj_float(c, data_R)

    Vectx = ch.naga_obj_float(c, np.zeros((sizem), dtype=np.float32))
    Vectx.random_host(seed * 5, 'U')

    Vecty = ch.naga_obj_float(c, np.zeros((sizem), dtype=np.float32))

    A = np.array(MatSym)

    x2 = np.array(Vectx)

    y = A.dot(x2)

    MatSym.symv(Vectx, vecty=Vecty)
    Vecty_2 = MatSym.symv(Vectx)

    npt.assert_array_almost_equal(y, np.array(Vecty), decimal=dec)
    npt.assert_array_almost_equal(y, np.array(Vecty_2), decimal=dec)
