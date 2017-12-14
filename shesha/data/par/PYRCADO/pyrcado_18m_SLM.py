import shesha_config as conf
import shesha_constants as scons

simul_name = ""

#loop
p_loop = conf.Param_loop()

p_loop.set_niter(1000)  #500Hz: 1mn = 30000, 1kH = 60000
p_loop.set_ittime(1 / 500.)  #=1/500

#geom
p_geom = conf.Param_geom()
p_geom.set_zenithangle(0.)

#tel
p_tel = conf.Param_tel()
p_tel.set_diam(18.)

#atmos
p_atmos = conf.Param_atmos()

p_atmos.set_r0(0.129)
p_atmos.set_nscreens(1)
p_atmos.set_frac([1.0])
p_atmos.set_alt([0.0])
p_atmos.set_windspeed([10.])
p_atmos.set_winddir([45.])
p_atmos.set_L0([25.])  # Not simulated in Yorick?

# No target

# Fake WFS for raytracing
p_wfs0 = conf.Param_wfs()
p_wfss = [p_wfs0]

p_wfs0.set_type(scons.WFSType.SH)

p_wfs0.set_nxsub(38)

p_wfs0.set_npix(4)
p_wfs0.set_pixsize(0.3)
p_wfs0.set_fracsub(0.8)
p_wfs0.set_xpos(0.)
p_wfs0.set_ypos(0.)
p_wfs0.set_Lambda(1.00)
p_wfs0.set_gsmag(15.5)
p_wfs0.set_optthroughput(0.5)
p_wfs0.set_zerop(1.e11)
p_wfs0.set_noise(-1)  # in electrons units
p_wfs0.set_atmos_seen(1)
p_wfs0.set_fstop(scons.FieldStopType.ROUND)
p_wfs0.set_fssize(2.4)

#dm
p_dm0 = conf.Param_dm()
p_dms = [p_dm0]
p_dm0.set_type(scons.DmType.PZT)
nact = p_wfs0.nxsub + 1

p_dm0.set_margin_out(0)

p_dm0.set_nact(nact)
p_dm0.set_alt(0.)
p_dm0.set_thresh(0.3)  # fraction units
p_dm0.set_coupling(
        0.2)  # !!!!!!!!!!!!!!!!!!!!!!!!! attention pas autre chose que 0.2 !!!!!!!!!
p_dm0.set_unitpervolt(1)
p_dm0.set_push4imat(1.)

# p_controller0 = conf.Param_controller()
# p_controllers = [p_controller0]
# p_controller0.set_ndm([0])
