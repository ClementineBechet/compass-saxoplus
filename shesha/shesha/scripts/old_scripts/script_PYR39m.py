import cProfile
import pstats as ps
import sys, os
sys.path.insert(0, os.environ["SHESHA_ROOT"] + "/widgets/")
sys.path.insert(0, os.environ["SHESHA_ROOT"] + "/lib/")

#from adoptLib import computeKLModesImat, computeCmatModal
from shesha.util import tools
import numpy as np
import carmaWrap as ch
import shesha as ao
import time
import matplotlib.pyplot as plt
import hdf5_utils as h5u
import resDataBase as db
import astropy.io.fits as pf
import glob
import pandas as pd

print("TEST SHESHA\n closed loop: call loop(int niter)")
simulName = "PYR_39m"
pathResults = "/volumes/hra/micado/PYR39m_RoundPupil_RUN2/"
dBResult = "/volumes/hra/micado/PYR_39m_RoundPupil_RUN2.h5"
#GPUS = np.array([0, 1, 2, 3])

if (len(sys.argv) == 1):
    error = 'command line should be:"python -i test.py parameters_filename"\n with "parameters_filename" the path to the parameters file'
    raise Exception(error)
if (len(sys.argv) == 2):
    print("Using Internal parameters...")
    """
    -----------------
            INPUTS
    -----------------
    """
    freqs = [500.]
    gainslist = [1]
    #magnitudes=[11, 12, 13, 14, 15, 16]
    magnitudes = [11, 15]
    nKL_Filt = 1000
    MODU = [5]
    RONS = [0.5]
else:
    print("DETECTED BASH SCRIPT")
    #python $script $PARFILE $FREQ $MODU $GAIN $MAG $KLFILT
    print(sys.argv)
    freqs = [float(sys.argv[2])]  # frequency
    RONS = [float(sys.argv[3])]  # RONS
    MODU = [float(sys.argv[4])]  # MODU
    gainslist = [float(sys.argv[5])]  # frequency
    magnitudes = [float(sys.argv[6])]  # frequency
    nKL_Filt = float(sys.argv[7])  # frequency

#$FREQ $NPIX $PIXSIZE $GAIN $TH $MAG $KLFILT
Nsimutot = len(gainslist) * len(freqs) * len(RONS) * len(MODU) * len(magnitudes)

if (not glob.glob(pathResults)):
    print("Results folder not found. Creating it now:")
    tools.system("mkdir " + pathResults)
if (not glob.glob(pathResults + "PSFs/")):
    print("PSFs folder not found. Creating it now:")

    tools.system("mkdir " + pathResults + "PSFs/")

#get parameters from file
param_file = sys.argv[1]  # par filename
if (param_file.split('.')[-1] == b"py"):
    filename = param_file.split('/')[-1]
    param_path = param_file.split(filename)[0]
    sys.path.insert(0, param_path)
    exec("import %s as config" % filename.split(".py")[0])
    #sys.path.remove(param_path)
elif (param_file.split('.')[-1] == b"h5"):
    sys.path.insert(0, os.environ["SHESHA_ROOT"] + "/data/par/par4bench/")
    import scao_sh_16x16_8pix as config
    #sys.path.remove(os.environ["SHESHA_ROOT"]+"/data/par/par4bench/")
    h5u.configFromH5(param_file, config)
else:
    raise ValueError("Parameter file extension must be .py or .h5")

print("param_file is", param_file)

if (hasattr(config, "simul_name")):
    if (config.simul_name is None):
        simul_name = ""
    else:
        simul_name = config.simul_name
else:
    simul_name = ""
print("simul name is", simul_name)

matricesToLoad = {}
if (simul_name == b""):
    clean = 1
else:
    clean = 0
    param_dict = h5u.params_dictionary(config)
    matricesToLoad = h5u.checkMatricesDataBase(os.environ["SHESHA_ROOT"] + "/data/",
                                               config, param_dict)
#initialisation:
#   context
c = ch.carmaWrap_context(devices=np.array([0, 1, 2, 3], dtype=np.int32))

#c.set_activeDevice(6)


def makeFITSHeader(filepath, df):
    hdulist = pf.open(filepath)  # read file
    header = hdulist[0].header
    names = np.sort(list(set(df))).tolist()
    for name in names:
        val = df[name][0]
        if (type(val) is list):
            value = ""
            for v in val:
                value += (str(v) + " ")
        elif (type(val) is np.ndarray):
            value = ""
            for v in val:
                value += (str(v) + " ")
        else:
            value = val
        header.set(name, value, '')
    hdulist.writeto(filepath, clobber=True)  # Save changes to file


def initSimu(config, c):
    #    wfs
    param_dict = h5u.params_dictionary(config)
    matricesToLoad = h5u.checkMatricesDataBase(os.environ["SHESHA_ROOT"] + "/data/",
                                               config, param_dict)
    print("->wfs")
    wfs, tel = ao.wfs_init(config.p_wfss, config.p_atmos, config.p_tel, config.p_geom,
                           config.p_target, config.p_loop, config.p_dms)
    #   atmos
    print("->atmos")
    atm = ao.atmos_init(c, config.p_atmos, config.p_tel, config.p_geom, config.p_loop,
                        config.p_wfss, wfs, config.p_target, rank=0, clean=clean,
                        load=matricesToLoad)

    #   dm
    print("->dm")
    dms = ao.dm_init(config.p_dms, config.p_wfss, wfs, config.p_geom, config.p_tel)

    #   target
    print("->target")
    tar = ao.target_init(c, tel, config.p_target, config.p_atmos, config.p_geom,
                         config.p_tel, config.p_dms)

    print("->rtc")
    #   rtc
    rtc = ao.rtc_init(tel, wfs, config.p_wfss, dms, config.p_dms, config.p_geom,
                      config.p_rtc, config.p_atmos, atm, config.p_tel, config.p_loop,
                      do_refslp=False, clean=clean, simul_name=simul_name,
                      load=matricesToLoad, doimat=0)

    h5u.validDataBase(os.environ["SHESHA_ROOT"] + "/data/", matricesToLoad)

    print("====================")
    print("init done")
    print("====================")
    print("objects initialzed on GPU:")
    print("--------------------------------------------------------")
    print(atm)
    print(wfs)
    print(dms)
    print(tar)
    print(rtc)
    return wfs, tel, atm, dms, tar, rtc


def loop(n, wfs, tel, atm, dms, tar, rtc):
    t0 = time.time()
    print("----------------------------------------------------")
    print("iter# | S.E. SR | L.E. SR | Est. Rem. | framerate")
    print("----------------------------------------------------")
    sr_se = []
    sr_le = []
    numiter = []
    for i in range(n):
        atm.move_atmos()

        if (config.p_controllers[0].type == b"geo"):
            for t in range(config.p_target.ntargets):
                tar.atmos_trace(t, atm, tel)
                rtc.do_control_geo(0, dms, tar, 0)
                rtc.apply_control(0)
                tar.dmtrace(0, dms)
        else:
            for t in range(config.p_target.ntargets):
                tar.atmos_trace(t, atm, tel)
                tar.dmtrace(t, dms)
            for w in range(len(config.p_wfss)):
                wfs.raytrace(w, "all", tel, atm, dms)
                wfs.sensors_compimg(w)

            rtc.do_centroids(0)
            rtc.docontrol(0)
            rtc.apply_control(0)

        if ((i + 1) % 100 == 0):
            print("Iter#:", i + 1)
            #for t in range(config.p_target.ntargets):
            t = 1
            SR = tar.get_strehl(t)
            print("Tar %d at %3.2fMicrons:" % (t + 1, tar.Lambda[t]))
            signal_se = "SR S.E: %1.2f   " % SR[0]
            signal_le = "SR L.E: %1.2f   " % SR[1]

            print(signal_se + signal_le)
            #print(i+1,"\t",,SR[0],"\t",SR[1])
            sr_le.append(SR[1])
            sr_se.append(SR[0])
            numiter.append(i + 1)


#
#        plt.pause(0.01)
#        plt.scatter(numiter, sr_le, color="green", label="Long Exposure")
#        plt.plot(numiter, sr_le, color="green")
#        plt.scatter(numiter, sr_se, color="red", label="Short Exposure")
#        plt.plot(numiter, sr_se, color="red")

    t1 = time.time()
    print(" loop execution time:", t1 - t0, "  (", n, "iterations), ", (t1 - t0) / n,
          "(mean)  ", n / (t1 - t0), "Hz")
    SRList = []
    for t in range(config.p_target.ntargets):
        SR = tar.get_strehl(t)
        SRList.append(SR[1])  # Saving Long Exp SR
    return SRList, tar.Lambda.tolist(), sr_le, sr_se, numiter

mimg = 0.  # initializing average image

SR = []
"""
dictProcess, dictplot = getDataFrameColumns()
colnames = h5u.params_dictionary(config)
resAll = pd.DataFrame( columns=colnames.keys()) # res is the local dataframe for THIS data set
resAll = resAll.append(colnames, ignore_index=True)  #Fill dataframe
resAll.srir = None
"""

colnames = h5u.params_dictionary(config)  # config values internal to compass
#simunames = {"PSFFilenames":None, "srir":None, "lambdaTarget":None, "threshold":None, "sr_le":None, "sr_se":None, "numiter":None, "NklFilt":None, "NklTot":None, "Nkl":None, "eigenvals":None, "Nphotons":None, "Nactu":None, "Nslopes":None}# Added values computed by the simu..
simunames = {
        "PSFFilenames": None,
        "srir": None,
        "lambdaTarget": None,
        "nbBrightest": None,
        "sr_le": None,
        "sr_se": None,
        "numiter": None,
        "NklFilt": None,
        "NklTot": None,
        "Nkl": None,
        "eigenvals": None,
        "Nphotons": None,
        "Nactu": None,
        "RON": None,
        "Nslopes": None
}  # Added values computed by the simu..

resAll = db.readDataBase(fullpath=dBResult)  # Reads all the database if exists
if (not (type(resAll) == pd.core.frame.DataFrame)):
    print("Creating compass database")
    resAll = db.createDf(list(colnames.keys()) + list(
            simunames.keys()))  # Creates the global compass Db

#res = db.addcolumn(res,simunames)

#freqs = [100.,300., 500., 1000.]
#npixs = [4,6,8]
#pixsizes = [0.5,1,1.5] # in lambda/dssp
#gainslist = [0.1, 0.3, 0.5]
#ths=[0,1,2,3]
#magnitudes=[11.5,12.5,13.5,14.5]
##

#res500 = pf.getdata("/home/fvidal/res_500.fits")
#fig = plt.figure(num = 1)
#fig.show()
NCurrSim = 0

for freq in freqs:
    config.p_loop.set_ittime(1 / freq)
    for RON in RONS:
        config.p_wfs0.set_noise(RON)
        for modulation in MODU:
            rMod = modulation
            config.p_wfs0.set_pyr_npts(
                    int(np.ceil(int(rMod * 2 * 3.141592653589793) / 4.) * 4))
            config.p_wfs0.set_pyr_ampl(rMod)
            for gain in gainslist:
                config.p_controller0.set_gain(gain)  # Change Gain
                for magnitude in magnitudes:
                    NCurrSim += 1
                    config.p_wfs0.set_gsmag(magnitude)
                    res = pd.DataFrame(
                            columns=list(colnames.keys()) + list(
                                    simunames.keys()))  # Create Db for last result
                    print("Freq = %3.2f Hz" % (1. / config.p_loop.ittime))
                    print("Magnitude = %3.2f" % config.p_wfs0.gsmag)
                    print("Gain = %3.2f" % config.p_controller0.gain)

                    wfs, tel, atm, dms, tar, rtc = initSimu(config, c)  # Init Simu
                    nfilt = nKL_Filt
                    #cmat = ao.compute_cmatWithKL(rtc, config.p_controllers[0], dms, config.p_dms, config.p_geom, config.p_atmos, config.p_tel, nfilt)
                    print("Reading cMat")
                    print(1 / 0.)

                    #cmat = pf.getdata(os.environ["SHESHA_ROOT"]+"/test/scripts/cmatKLGood.fits").byteswap().newbyteorder()
                    print("Setting cMat")
                    #rtc.set_cmat(0, cmat.copy().astype(np.float32))
                    rtc.set_cmat(0, cmat.copy())
                    print("Starting Loop")
                    SR, lambdaTargetList, sr_le, sr_se, numiter = loop(
                            config.p_loop.niter, wfs, tel, atm, dms, tar, rtc)
                    dfparams = h5u.params_dictionary(
                            config)  # get the current compass config
                    dfparams.update(simunames)  # Add the simunames params

                    res = db.fillDf(res, dfparams)  # Saving dictionnary config
                    res.loc[0, "NklFilt"] = nKL_Filt
                    res.loc[0, "Nkl"] = cmat.shape[0] - 2 - nKL_Filt
                    res.loc[0, "NklTot"] = cmat.shape[0] - 2
                    res.loc[0, "Nactu"] = cmat.shape[0]
                    res.loc[0, "Nslopes"] = cmat.shape[1]
                    res.loc[0, "Nphotons"] = config.p_wfs0._nphotons
                    res.loc[0, "RON"] = RON
                    res.loc[0, "Nphotons"] = config.p_wfs0._nphotons
                    #res.eigenvals.values[0] = rtc.getEigenvals(0)
                    res.srir.values[0] = SR  # Saving computed values
                    res.lambdaTarget.values[0] = lambdaTargetList
                    res.loc[0, "gsmag"] = config.p_wfs0.gsmag
                    res.loc[0, "gain"] = config.p_controller0.gain
                    #res.sr_le.values[0] = sr_le
                    #res.sr_se.values[0] = sr_se
                    #res.numiter.values[0] = numiter
                    res.loc[0, "simulname"] = simulName
                    print("Saving PSFs...")
                    PSFNameList = []
                    for t in range(config.p_target.ntargets):
                        PSFtarget = tar.get_image(t, "le")
                        date = time.strftime("_%d-%m-%Y_%H:%M:%S_")
                        lam = "%3.2f" % tar.Lambda.tolist()[t]
                        lam = lam.replace(".", "_")
                        PSFName = "PYR_" + lam + "_" + date + ".fits"
                        PSFNameList.append(PSFName)
                        #PSFNameList.append("NOT SAVED")
                        pf.writeto(pathResults + "PSFs/" + PSFName, PSFtarget.copy(),
                                   clobber=True)
                        lam2 = "%3.2f" % tar.Lambda.tolist()[t]
                        res.loc[0, "SR_%s" % lam2] = SR[t]
                        filepath = pathResults + "PSFs/" + PSFName

                        #"Add the SR and wavelegth value at the top of the PSF header file"
                        hdulist = pf.open(filepath)  # read file
                        header = hdulist[0].header
                        header["SR"] = SR[t]
                        header["wavelength"] = tar.Lambda.tolist()[t]
                        hdulist.writeto(filepath, clobber=True)  # Save changes to file
                        # Adding all the parameters to the header
                        makeFITSHeader(filepath, res)
                    print("Done")
                    res.loc[0, "type_ap"] = str(res.loc[0, "type_ap"][0])
                    res.loc[0, "type"] = str(res.loc[0, "type"][0])
                    res.loc[0, "type"] = "pzt, tt"
                    res.loc[0, "npix"] = res.loc[0, "npix"][0]
                    #res.loc[0, "nbBrightest"] = res.loc[0, "nbBrightest"][0]
                    res.loc[0, "pixsize"] = res.loc[0, "pixsize"][0]
                    res.PSFFilenames.values[0] = PSFNameList
                    resAll = db.fillDf(resAll, res)  # Saving in global DB
                    #resAll.to_hdf("/home/fvidal/compass/trunk/shesha/test/scripts/resultatsScripts/SH39m.h5", "resAll", complevel=9,complib='blosc')
                    resAll.to_hdf(dBResult, "resAll", complevel=9, complib='blosc')
                    #db.saveDataBase(resAll)

print("Simulation Done...")
"""
Sauver PSF dans le bon nom + directory
 ranger... + params dans le header

 SR = np.zeros((3, len(set(resAll.gsmag))))
 i=0
 for mag in list(set(resAll.gsmag)):
     SR[0,i] = resAll[resAll.gsmag == mag]["SR_1.20"].max()
     SR[1,i] = resAll[resAll.gsmag == mag]["SR_1.65"].max()
     SR[2,i] = resAll[resAll.gsmag == mag]["SR_2.20"].max()
     i+=1


"""
