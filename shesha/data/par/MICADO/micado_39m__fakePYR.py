import shesha.config as conf
import numpy as np

simul_name = "fake_pyr"

# loop
p_loop = conf.ParamLoop()

p_loop.set_niter(1000)
p_loop.set_ittime(1 / 500.0)  # =1/500
p_loop.set_devices([4])
# geom
p_geom = conf.ParamGeom()
p_geom.set_zenithangle(0.0)

# tel
p_tel = conf.ParamTel()
p_tel.set_diam(39.0)
p_tel.set_cobs(0.28)

# E_ELT PUPIL
"""
p_tel.set_type_ap("EELT-Nominal")
#p_tel.set_spiders_type("six")
p_tel.set_pupangle(0)
"""
"""
p_tel.set_nbrmissing(7)
p_tel.set_referr(0.01)
p_tel.set_std_tt(0.050) # microns
p_tel.set_std_piston(0.050) # microns
"""

# atmos
p_atmos = conf.ParamAtmos()

# p_atmos.set_r0(0.129)
p_atmos.set_r0(0.129)
p_atmos.set_nscreens(1)
p_atmos.set_frac([1.0])
p_atmos.set_alt([0.0])
p_atmos.set_windspeed([10.0])
p_atmos.set_winddir([45.0])
p_atmos.set_L0([25.0])  # Not simulated in Yorick?

# target
p_target = conf.ParamTarget()
p_targets = [p_target]
p_target.set_xpos(0)
p_target.set_ypos(0.0)
p_target.set_Lambda(1.65)
p_target.set_mag(4.0)

# wfs
# p_wfs0 = conf.ParamWfs(roket=True)
p_wfs0 = conf.ParamWfs()
p_wfss = [p_wfs0]

p_wfs0.set_type("pyrhr")
p_wfs0.set_nxsub(88)
p_wfs0.set_fracsub(0.8)
p_wfs0.set_xpos(0.0)
p_wfs0.set_ypos(0.0)
p_wfs0.set_Lambda(0.7)
p_wfs0.set_gsmag(11)
p_wfs0.set_optthroughput(0.5)
p_wfs0.set_zerop(2.6e10)  # 2.6e10 ph/s/m**2 computed by Rico in R band for MOSAIC
p_wfs0.set_noise(-1)  # in electrons units
p_wfs0.set_atmos_seen(1)
p_wfs0.set_fstop("square")
p_wfs0.set_fssize(1.6)
rMod = 5
p_wfs0.set_pyr_npts(int(np.ceil(int(rMod * 2 * 3.141592653589793) / 4.0) * 4))
# p_wfs0.set_pyr_npts(31)
p_wfs0.set_pyr_ampl(rMod)
# p_wfs0.set_pyr_pup_sep(int(2 / 3. * p_wfs0.nxsub)) # diffraction effect
# p_wfs0.set_pyr_pup_sep((p_wfs0.nxsub))
p_wfs0.set_pyr_pup_sep((64))

# dm
p_dm0 = conf.ParamDm()
p_dm1 = conf.ParamDm()
p_dms = [p_dm0, p_dm1]
p_dm0.set_type("pzt")
nact = p_wfs0.nxsub + 1
# nact = 9

p_dm0.set_nact(nact)
p_dm0.set_alt(0.0)
p_dm0.set_thresh(0.3)  # fraction units
# !!!!!!!!!!!!!!!!!!!!!!!!! attention pas autre chose que 0.2 !!!!!!!!!
p_dm0.set_coupling(0.2)
p_dm0.set_unitpervolt(1)
p_dm0.set_push4imat(1)
# p_dm0.set_gain(0.2)

p_dm1.set_type("tt")
p_dm1.set_alt(0.0)
p_dm1.set_unitpervolt(1)
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

p_controller0.set_type("ls")

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
# p_rtc = conf.Param_rtc()
#
# p_rtc.set_nwfs(1)
# p_rtc.set_centroiders(p_centroiders)
# p_rtc.set_controllers(p_controllers)
