
import cProfile
import pstats as ps

import sys
import numpy as np
import chakra as ch
import chakra_ao as ao
import time

p_geom=ao.Param_geom()
p_atmos=ao.Param_atmos()
p_tel=ao.Param_tel()
p_target=ao.Param_target()
p_loop = ao.Param_loop()
p_wfs= ao.Param_wfs()
p_wfs1= ao.Param_wfs()
p_wfs2= ao.Param_wfs()


#loop
p_loop.set_niter(2000)
p_loop.set_ittime(0.002) #=1/500
#geom
p_geom.set_zenithangle(0.)

#tel
p_tel.set_diam(40.)
p_tel.set_cobs(0.30)


#atmos
p_atmos.set_r0(0.16)
p_atmos.set_nscreens(1)
p_atmos.set_frac([1.0])
p_atmos.set_alt([0.0])
p_atmos.set_windspeed([20.0])
p_atmos.set_winddir([45])
p_atmos.set_L0([1.e3])

#target
p_target.set_nTargets(1)
p_target.set_xpos([0])
p_target.set_ypos([0.])
p_target.set_Lambda([1.65])
p_target.set_mag([10])

#wfs
p_wfs.set_type("sh")
p_wfs.set_nxsub(80)
p_wfs.set_npix(8)#8#20
p_wfs.set_pixsize(0.3)#0.3#1.0
p_wfs.set_fracsub(0.8)
p_wfs.set_xpos(0.)
p_wfs.set_ypos(0.)
p_wfs.set_Lambda(0.5)
p_wfs.set_gsmag(5.)
p_wfs.set_optthroughput(0.5)
p_wfs.set_zerop(1.e11)
p_wfs.set_noise(-1)
p_wfs.set_atmos_seen(1)
p_wfs.set_zerop(1e11)

c=ch.chakra_context()
c.set_activeDevice(0%c.get_ndevice())

#initialisation:
#    wfs
list_wfs=[p_wfs]
wfs=ao.wfs_init(list_wfs,p_atmos,p_tel,p_geom,p_loop,1,0)
#   atmos
atm=p_atmos.atmos_init(c,p_tel,p_geom,p_loop)
#   target
tar=p_target.target_init(c,p_atmos,p_geom,p_tel,list_wfs,wfs,p_tel)

start = time.time()
#generate turbulence, raytrace through the atmos
# and display:
#   the turbulence
#   the target's image 
#ao.see_atmos_target(5,atm,tar,alt=0,n_tar=0,f=0.15)
ao.see_atmos_target(100,atm,tar,wfs,alt=0,n_tar=0,f=0.15)

#for profiling purpose
#filename="Profile.prof"
#cProfile.runctx("ao.see_atmos_target(10,atm,tar,wfs,alt=0,n_tar=0,f=0.15)", globals(), locals(),filename)

end = time.time()

#for profiling purpose
#s = ps.Stats(filename)
#s.strip_dirs().sort_stats("time").print_stats()

end = time.time()
print end - start


shak=wfs.get_binimg(0)
screen=atm.get_screen(0)
img=tar.get_image(0,"se")
print shak.shape, screen.shape, img.shape
np.save("wfs_binimg",shak)
np.save("target_image",img)
np.save("atmos_screen",screen)
#use to compare result with non mpi exec
# load numpy array with numpy.load("wfs_binimg.npy")
