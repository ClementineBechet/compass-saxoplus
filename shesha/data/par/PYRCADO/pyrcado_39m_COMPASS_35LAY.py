import shesha.config as conf
import numpy as np

simul_name = ""

# loop
p_loop = conf.ParamLoop()

p_loop.set_niter(1000)
p_loop.set_ittime(1 / 500.0)  # =1/500
p_loop.set_devices([0, 1, 2])

# geom
p_geom = conf.ParamGeom()
p_geom.set_zenithangle(0.0)
p_geom.set_pupdiam(500)

# tel
p_tel = conf.ParamTel()
p_tel.set_diam(39.0)

# -----------------------------
# 1 layer profile
# -----------------------------

r01L = 0.129
alt1L = [0.0]
frac1L = [1.0]
wind1L = [10.0]

# -----------------------------
# 35 layers profile
# -----------------------------

r0med = 0.144

r0Q1 = 0.2147
r0Q2 = 0.1633
r0Q3 = 0.1275
r0Q4 = 0.089

zenithAngle = 30.0
altESO = np.array(
    [
        30,
        90,
        150,
        200,
        245,
        300,
        390,
        600,
        1130,
        1880,
        2630,
        3500,
        4500,
        5500,
        6500,
        7500,
        8500,
        9500,
        10500,
        11500,
        12500,
        13500,
        14500,
        15500,
        16500,
        17500,
        18500,
        19500,
        20500,
        21500,
        22500,
        23500,
        24500,
        25500,
        26500,
    ]
) / np.cos(zenithAngle * 2 * np.pi / 360)
altESO = altESO.astype(int)

fracmed = [
    24.2,
    12,
    9.68,
    5.9,
    4.73,
    4.73,
    4.73,
    4.73,
    3.99,
    3.24,
    1.62,
    2.6,
    1.56,
    1.04,
    1,
    1.2,
    0.4,
    1.4,
    1.3,
    0.7,
    1.6,
    2.59,
    1.9,
    0.99,
    0.62,
    0.4,
    0.25,
    0.22,
    0.19,
    0.14,
    0.11,
    0.06,
    0.09,
    0.05,
    0.04,
]
fracQ1 = [
    22.6,
    11.2,
    10.1,
    6.4,
    4.15,
    4.15,
    4.15,
    4.15,
    3.1,
    2.26,
    1.13,
    2.21,
    1.33,
    0.88,
    1.47,
    1.77,
    0.59,
    2.06,
    1.92,
    1.03,
    2.3,
    3.75,
    2.76,
    1.43,
    0.89,
    0.58,
    0.36,
    0.31,
    0.27,
    0.2,
    0.16,
    0.09,
    0.12,
    0.07,
    0.06,
]
fracQ2 = [
    25.1,
    11.6,
    9.57,
    5.84,
    3.7,
    3.7,
    3.7,
    3.7,
    3.25,
    3.47,
    1.74,
    3,
    1.8,
    1.2,
    1.3,
    1.56,
    0.52,
    1.82,
    1.7,
    0.91,
    1.87,
    3.03,
    2.23,
    1.15,
    0.72,
    0.47,
    0.3,
    0.25,
    0.22,
    0.16,
    0.13,
    0.07,
    0.11,
    0.06,
    0.05,
]
fracQ3 = [
    25.5,
    11.9,
    9.32,
    5.57,
    4.5,
    4.5,
    4.5,
    4.5,
    4.19,
    4.04,
    2.02,
    3.04,
    1.82,
    1.21,
    0.86,
    1.03,
    0.34,
    1.2,
    1.11,
    0.6,
    1.43,
    2.31,
    1.7,
    0.88,
    0.55,
    0.36,
    0.22,
    0.19,
    0.17,
    0.12,
    0.1,
    0.06,
    0.08,
    0.04,
    0.04,
]
fracQ4 = [
    23.6,
    13.1,
    9.81,
    5.77,
    6.58,
    6.58,
    6.58,
    6.58,
    5.4,
    3.2,
    1.6,
    2.18,
    1.31,
    0.87,
    0.37,
    0.45,
    0.15,
    0.52,
    0.49,
    0.26,
    0.8,
    1.29,
    0.95,
    0.49,
    0.31,
    0.2,
    0.12,
    0.1,
    0.09,
    0.07,
    0.06,
    0.03,
    0.05,
    0.02,
    0.02,
]

windESO = [
    5.5,
    5.5,
    5.1,
    5.5,
    5.6,
    5.7,
    5.8,
    6,
    6.5,
    7,
    7.5,
    8.5,
    9.5,
    11.5,
    17.5,
    23,
    26,
    29,
    32,
    27,
    22,
    14.5,
    9.5,
    6.3,
    5.5,
    6,
    6.5,
    7,
    7.5,
    8,
    8.5,
    9,
    9.5,
    10,
    10,
]

# atmos
p_atmos = conf.ParamAtmos()

r0 = r0med
alt = altESO
frac = fracmed
wind = windESO

nbLayers = len(alt)
# p_atmos.set_r0(0.129)
p_atmos.set_r0(r0)
p_atmos.set_nscreens(nbLayers)
p_atmos.set_frac(frac)
p_atmos.set_alt(alt)
p_atmos.set_windspeed(wind)
p_atmos.set_winddir([45.0] * nbLayers)
p_atmos.set_L0([25.0] * nbLayers)  # Not simulated in Yorick?

# Lambda target(s)
p_targets = [conf.ParamTarget() for _ in range(2)]
Lambda = [0.658, 1.65]
k = 0
for p_target in p_targets:
    p_target.set_xpos(0.0)
    p_target.set_ypos(0.0)
    p_target.set_Lambda(Lambda[k])
    p_target.set_mag(4.0)
    k += 1

# wfs
p_wfs0 = conf.ParamWfs()
p_wfss = [p_wfs0]

p_wfs0.set_type("pyrhr")
p_wfs0.set_nxsub(78)
p_wfs0.set_fracsub(0.01)
p_wfs0.set_xpos(0.0)
p_wfs0.set_ypos(0.0)
p_wfs0.set_Lambda(0.658)
p_wfs0.set_gsmag(13)
p_wfs0.set_optthroughput(0.5)
p_wfs0.set_zerop(2.6e10)  # 2.6e10 ph/s/m**2 computed by Rico in R band for MOSAIC
p_wfs0.set_noise(-1)  # in electrons units
p_wfs0.set_atmos_seen(1)
p_wfs0.set_fstop("square")
p_wfs0.set_fssize(1.6)

rMod = 8

p_wfs0.set_pyr_npts(int(np.ceil(int(rMod * 2 * 3.141592653589793) / 4.0) * 4))
p_wfs0.set_pyr_ampl(rMod)
p_wfs0.set_pyr_pup_sep((64))

# dm
p_dm0 = conf.ParamDm()
p_dm1 = conf.ParamDm()
p_dms = [p_dm0, p_dm1]
p_dm0.set_type("pzt")
nact = p_wfs0.nxsub + 1
p_dm0.set_margin_out(0)
p_dm0.set_nact(nact)
p_dm0.set_alt(0.0)
p_dm0.set_thresh(0.3)  # fraction units
# !!!!!!!!!!!!!!!!!!!!!!!!! attention pas autre chose que 0.2 !!!!!!!!!
p_dm0.set_coupling(0.2)
p_dm0.set_unitpervolt(1)
p_dm0.set_push4imat(1.0)
# p_dm0.set_gain(0.2)

p_dm1.set_type("tt")
p_dm1.set_alt(0.0)
p_dm1.set_unitpervolt(1.0)
p_dm1.set_push4imat(0.005)
# p_dm1.set_gain(0.2)

# centroiders
p_centroider0 = conf.ParamCentroider()
p_centroiders = [p_centroider0]

p_centroider0.set_nwfs(0)
p_centroider0.set_type("pyr")
# sp_centroider0.set_thresh(0.)

# p_centroider0.set_type("bpcog")
# p_centroider0.set_nmax(10)

# p_centroider0.set_type("corr")
# p_centroider0.set_type_fct("model")

# controllers
p_controller0 = conf.ParamController()
p_controllers = [p_controller0]

# p_controller0.set_type("ls")
p_controller0.set_type("generic")

p_controller0.set_nwfs([0])
p_controller0.set_ndm([0, 1])
p_controller0.set_maxcond(150.0)
p_controller0.set_delay(1)
p_controller0.set_gain(1)

# p_controller0.set_modopti(0)
# p_controller0.set_nrec(2048)
# p_controller0.set_nmodes(5064)
# p_controller0.set_gmin(0.001)
# p_controller0.set_gmax(0.5)
# p_controller0.set_ngain(500)

# rtc
p_rtc = conf.Param_rtc()

p_rtc.set_nwfs(1)
p_rtc.set_centroiders(p_centroiders)
p_rtc.set_controllers(p_controllers)
