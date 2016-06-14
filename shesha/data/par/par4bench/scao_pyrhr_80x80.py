import shesha as ao

#simul_name="bench_scao_pyr40_8pix"

#loop
p_loop = ao.Param_loop()

p_loop.set_niter(1000)
p_loop.set_ittime(0.002) #=1/500


#geom
p_geom=ao.Param_geom()

p_geom.set_zenithangle(0.)


#tel
p_tel=ao.Param_tel()

p_tel.set_diam(40.0)
p_tel.set_cobs(0.3)


#atmos
p_atmos=ao.Param_atmos()

p_atmos.set_r0(0.16)
p_atmos.set_nscreens(1)
p_atmos.set_frac([1.0])
p_atmos.set_alt([0.0])
p_atmos.set_windspeed([10.0])
p_atmos.set_winddir([0])
p_atmos.set_L0([100.])


#target
p_target=ao.Param_target()

p_target.set_nTargets(1)
p_target.set_xpos([0])
p_target.set_ypos([0.])
p_target.set_Lambda([1.6])
p_target.set_mag([10])


#wfs
p_wfs0= ao.Param_wfs()
p_wfss=[p_wfs0]


p_wfs0.set_type("pyrhr")
p_wfs0.set_nxsub(80)
p_wfs0.set_fssize(1.5)
p_wfs0.set_fracsub(1)
p_wfs0.set_xpos(0.)
p_wfs0.set_ypos(0.)
p_wfs0.set_Lambda(0.5)
p_wfs0.set_gsmag(5.)
p_wfs0.set_optthroughput(0.5)
p_wfs0.set_zerop(1.e11)
p_wfs0.set_noise(-1)
p_wfs0.set_fstop("round")
p_wfs0.set_pyr_npts(16)
p_wfs0.set_pyr_ampl(3)
p_wfs0.set_atmos_seen(1)


#dm
p_dm0=ao.Param_dm()
p_dm1=ao.Param_dm()
p_dms=[p_dm0,p_dm1]
p_dm0.set_type("pzt")
nact=p_wfs0.nxsub+1
p_dm0.set_nact(nact)
p_dm0.set_alt(0.)
p_dm0.set_thresh(0.3)
p_dm0.set_coupling(0.2)
p_dm0.set_unitpervolt(0.01)
p_dm0.set_push4imat(100.)

p_dm1.set_type("tt")
p_dm1.set_alt(0.)
p_dm1.set_unitpervolt(0.0005)
p_dm1.set_push4imat(100)



#centroiders
p_centroider0=ao.Param_centroider()
p_centroiders=[p_centroider0]

p_centroider0.set_nwfs(0)
p_centroider0.set_type("pyr")
#p_centroider0.set_type("corr")
#p_centroider0.set_type_fct("model")

#controllers
p_controller0=ao.Param_controller()
p_controllers=[p_controller0]

p_controller0.set_type("ls")
p_controller0.set_nwfs([0])
p_controller0.set_ndm([0,1])
p_controller0.set_maxcond(1000.)
p_controller0.set_delay(1)
p_controller0.set_gain(0.4)

#rtc
p_rtc=ao.Param_rtc()

p_rtc.set_nwfs(1)
p_rtc.set_centroiders(p_centroiders)
p_rtc.set_controllers(p_controllers)

