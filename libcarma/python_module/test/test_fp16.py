import carmaWrap
import numpy as np

c = carmaWrap.carmaWrap_context.get_instance()

m = 5000
n = 24000

A = np.random.random((m, n))
x = np.random.random((n, 1))
y = np.zeros((m, 1))

mat = carmaWrap.make_carmaWrap_obj_half(c, A)
matF = carmaWrap.carmaWrap_obj_float(c, A)
X = carmaWrap.make_carmaWrap_obj_half(c, x)
Y = carmaWrap.make_carmaWrap_obj_half(c, y)
XF = carmaWrap.carmaWrap_obj_float(c, x)
YF = carmaWrap.carmaWrap_obj_float(c, y)
