
import cProfile
import pstats as ps

import sys
import numpy as np
import chakra as ch
import chakra_ao as ao
import time

print "TEST CHAKRA_AO\n closed loop: call loop(int niter)"


if(len(sys.argv)!=2):
    error= 'command line should be:"python -i test.py parameters_filename"\n with "parameters_filename" the path to the parameters file'
    raise StandardError(error)

#get parameters from file
param_file=sys.argv[1]
execfile(param_file)

#initialisation:
#   context
c=ch.chakra_context()
c.set_activeDevice(0)

#    wfs
print "->wfs"
wfs=ao.wfs_init(p_wfss,p_atmos,p_tel,p_geom,p_target,p_loop, 1,0,p_dms)

#   atmos
print "->atmos"
atm=p_atmos.atmos_init(c,p_tel,p_geom,p_loop)

#   dm 
print "->dm"
dms=ao.dm_init(p_dms,p_wfs0,p_geom,p_tel)

#   target
print "->target"
tar=p_target.target_init(c,p_atmos,p_geom,p_tel,p_wfss,wfs,p_dms)

print "->rtc"
#   rtc
rtc=ao.rtc_init(wfs,p_wfss,dms,p_dms,p_geom,p_rtc,p_atmos,atm,p_loop,p_target)

print "===================="
print "init done"
print "===================="
print "objects initialzed on GPU:"
print "--------------------------------------------------------"
print atm
print wfs
print dms
print tar
print rtc

mimg = 0.# initializing average image
print "----------------------------------------------------";
print "iter# | S.E. SR | L.E. SR | Est. Rem. | framerate";
print "----------------------------------------------------";

import matplotlib.pyplot as pl

def loop( n):
    fig,((turbu,image),(shak,defMir))=pl.subplots(2,2, figsize=(15,15))
    pl.ion()
    pl.show()
    for i in range(n):
        atm.move_atmos()
        

        for t in range(p_target.ntargets):
            tar.atmos_trace(t,atm)
            tar.dmtrace(t,dms)

        for w in range(len(p_wfss)):
            wfs.sensors_trace(w,"all",atm,dms)
            wfs.sensors_compimg(w)

        rtc.docentroids(0)
        rtc.docontrol(0)
        rtc.applycontrol(0,dms)


        if((i+1)%50==0):
            turbu.clear()
            image.clear()
            shak.clear()
            defMir.clear()

            screen=atm.get_screen(0.)
            f1=turbu.matshow(screen,cmap='Blues_r')

            im=tar.get_image(0,"se")
            im=np.roll(im,im.shape[0]/2,axis=0)
            im=np.roll(im,im.shape[1]/2,axis=1)
            f2=image.matshow(im,cmap='Blues_r')

            sh=wfs.get_binimg(0)
            f3=shak.matshow(sh,cmap='Blues_r')
            
            dm=dms.get_dm("pzt",0.)
            f4=defMir.matshow(dm)

            pl.draw()

            strehltmp = tar.get_strehl(0)
            print i+1,"\t",strehltmp[0],"\t",strehltmp[1]




            #strehl ~ 0.7  (best=1)


"""
          strehltmp = target_getstrehl(g_target,0);
          grow,strehlsp,strehltmp(1);
          grow,strehllp,strehltmp(2);
          write,format=" %5i    %5.2f     %5.2f     %5.2f s   %5.2f it./s\n",
          cc,strehlsp(0),strehllp(0),(y_loop.niter - cc)*time_move, -1/timetmp*subsample;
"""

loop(p_loop.niter)
