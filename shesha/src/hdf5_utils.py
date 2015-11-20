import h5py
import pandas
import os
import numpy as np
from subprocess import check_output

def params_dictionary(config):
    """ Create and returns a dictionary of all the config parameters with the
    corresponding keys for further creation of database and save files
    
    :param config: (module) : simulation parameters
    :return param_dict: (dictionary) : dictionary of parameters
    """
    param_dict= {"simulname":config.simul_name,
        "revision":check_output("svnversion").replace("\n",""),
        # Loop params
        "niter":config.p_loop.niter,
        "ittime":config.p_loop.ittime,  
        # Geom params
        "zenithangle":config.p_geom.zenithangle,
        "pupdiam":config.p_geom.pupdiam,        
        # Telescope params
        "tel_diam":config.p_tel.diam,
        "cobs":config.p_tel.cobs,        
        # Atmos params
        "r0":config.p_atmos.r0,
        "nscreens":config.p_atmos.nscreens,
        "frac":config.p_atmos.frac,
        "atm.alt":config.p_atmos.alt,
        "windspeed":config.p_atmos.windspeed,
        "winddir":config.p_atmos.winddir,
        "L0":config.p_atmos.L0,   
        "seeds":config.p_atmos.seeds,           
        # Target params
        "ntargets":config.p_target.ntargets,
        "target.xpos":config.p_target.xpos,
        "target.ypos":config.p_target.ypos,
        "target.Lambda":config.p_target.Lambda,
        "target.mag":config.p_target.mag,
        "target.dms_seen":config.p_target.dms_seen}

    # WFS params
    if(config.p_wfss is not None):
        wfs_dict = {"nwfs":len(config.p_wfss),
            "type_wfs":[wfs.type_wfs for wfs in config.p_wfss],
            "nxsub":[wfs.nxsub for wfs in config.p_wfss],
            "npix":[wfs.npix for wfs in config.p_wfss],
            "pixsize":[wfs.pixsize for wfs in config.p_wfss],
            "fracsub":[wfs.fracsub for wfs in config.p_wfss],
            "wfs.xpos":[wfs.xpos for wfs in config.p_wfss],
            "wfs.ypos":[wfs.ypos for wfs in config.p_wfss],
            "wfs.Lambda":[wfs.Lambda for wfs in config.p_wfss],
            "gsmag":[wfs.gsmag for wfs in config.p_wfss],
            "optthroughput":[wfs.optthroughput for wfs in config.p_wfss],
            "zerop":[wfs.zerop for wfs in config.p_wfss],
            "noise":[wfs.noise for wfs in config.p_wfss],
            "atmos_seen":[wfs.atmos_seen for wfs in config.p_wfss],
            "dms_seen":[wfs.dms_seen if(wfs.dms_seen is not None) else np.arange(len(config.p_dms),dtype=np.int32) for wfs in config.p_wfss],
            "beamsize":[wfs.beamsize for wfs in config.p_wfss],
            "fssize":[wfs.fssize for wfs in config.p_wfss],
            "fstop":[wfs.fstop if(wfs.fstop) else "" for wfs in config.p_wfss],
            "gsalt":[wfs.gsalt for wfs in config.p_wfss],
            "laserpower":[wfs.laserpower for wfs in config.p_wfss],
            "lgsreturnperwatt":[wfs.lgsreturnperwatt for wfs in config.p_wfss],
            "lltx":[wfs.lltx for wfs in config.p_wfss],
            "llty":[wfs.llty for wfs in config.p_wfss],
            "openloop":[wfs.openloop for wfs in config.p_wfss],
            "proftype":[wfs.proftype if(wfs.proftype) else "" for wfs in config.p_wfss],
            "pyr_ampl":[wfs.pyr_ampl for wfs in config.p_wfss],
            "pyr_loc":[wfs.pyr_loc if(wfs.pyr_loc) else ""  for wfs in config.p_wfss],
            "pyr_npts":[wfs.pyr_npts for wfs in config.p_wfss],
            "pyrtype":[wfs.pyrtype if(wfs.pyrtype) else "" for wfs in config.p_wfss]}
    else:
        wfs_dict = {"nwfs":len(config.p_wfss),"type_wfs":None,"nxsub":None,"npix":None,
            "pixsize":None,"fracsub":None,"wfs.xpos":None,"wfs.ypos":None,"wfs.Lambda":None,
            "gsmag":None,"optthroughput":None,"zerop":None,"noise":None,
            "atmos_seen":None,"dms_seen":None,"beamsize":None,"fssize":None,
            "fstop":None,"gsalt":None,"laserpower":None,"lgsreturnperwatt":None,"lltx":None,"llty":None,
            "openloop":None,"proftype":None,"pyr_ampl":None,"pyr_loc":None,"pyr_npts":None,
            "pyrtype":None}
    param_dict.update(wfs_dict)
        
    # Dms params
    if(config.p_dms is not None):
        dms_dict = {"ndms":len(config.p_dms),
            "type_dm":[dm.type_dm for dm in config.p_dms],
            "dm.alt":[dm.alt for dm in config.p_dms],
            "coupling":[dm.coupling for dm in config.p_dms],
            "hyst":[dm.hyst for dm in config.p_dms],
            "margin":[dm.margin for dm in config.p_dms],
            "nact":[dm.nact for dm in config.p_dms],
            "nkl":[dm.type_dm for dm in config.p_dms],
            "pupoffset":[dm.pupoffset if(dm.pupoffset) else 0 for dm in config.p_dms],
            "push4imat":[dm.push4imat for dm in config.p_dms],
            "dm.thresh":[dm.thresh for dm in config.p_dms],
            "unitpervolt":[dm.unitpervolt for dm in config.p_dms]}
    else:
        dms_dict = {"ndms":len(config.p_dms),"type_dm":None,"dm.alt":None,
            "coupling":None,"hyst":None,"margin":None,"nact":None,"nkl":None,
            "pupoffset":None,"push4imat":None,"dm.thresh":None,"unitpervolt":None}
        
    param_dict.update(dms_dict)
    
    # Centroider params
    if(config.p_centroiders is not None):
        centro_dict={"ncentroiders":len(config.p_centroiders),
            "type_centro":[c.type_centro for c in config.p_centroiders],
            "nmax":[c.nmax for c in config.p_centroiders],
            "centro.nwfs":[c.nwfs for c in config.p_centroiders],
            "sizex":[c.sizex for c in config.p_centroiders],
            "sizey":[c.sizey for c in config.p_centroiders],
            "centroider.thresh":[c.thresh for c in config.p_centroiders],
            "type_fct":[c.type_fct if(c.type_fct) else "" for c in config.p_centroiders],
            "weights":[c.weights if(c.weights) else float(0) for c in config.p_centroiders],
            "width":[c.width for c in config.p_centroiders]}
    else:
        centro_dict={"ncentroiders":len(config.p_centroiders),"type_centro":None,
            "nmax":None,"centro.nwfs":None,"sizex":None,"sizey":None,
            "centroider.thresh":None,"type_fct":None,"weights":None,"width":None}
    param_dict.update(centro_dict)
        
         
    # Controller params
    if(config.p_controllers is not None):
        control_dict={"ncontrollers":len(config.p_controllers),
            "type_control":[c.type_control for c in config.p_controllers],
            "TTcond":[c.TTcond for c in config.p_controllers],
            "cured_ndivs":[c.cured_ndivs for c in config.p_controllers],
            "delay":[c.delay for c in config.p_controllers],
            "gain":[c.gain for c in config.p_controllers],
            "maxcond":[c.maxcond for c in config.p_controllers],
            "modopti":[c.modopti for c in config.p_controllers],
            #"nactu":[c.nactu for c in config.p_controllers],
            "ndm":[c.ndm for c in config.p_controllers],
            "nmodes":[c.nmodes for c in config.p_controllers],
            "nrec":[c.nrec for c in config.p_controllers],
            "gmin":[c.gmin for c in config.p_controllers],
            "gmax":[c.gmax for c in config.p_controllers],
            "ngain":[c.ngain for c in config.p_controllers],
            #"nvalid":[c.nvalid for c in config.p_controllers],
            "control.nwfs":[c.nwfs for c in config.p_controllers]}
    else:
        control_dict={"ncontrollers":len(config.p_controllers),"type_control":None,
            "TTcond":None,"cured_ndivs":None,"delay":None,"gain":None,
            "maxcond":None,"modopti":None,#"nactu":None,
            "ndm":None,"nmodes":None,
            "nrec":None,#"nvalid":None,
            "control.nwfs":None}
    
    param_dict.update(control_dict)
         
    return param_dict
    
def create_file_attributes(filename,param_dict):
    """ create_file_attributes(filename,config)
    Create an hdf5 file wtih attributes corresponding to all simulation parameters
    
    :param:
        filename : (str) : full path + filename to create
        config : () : simulation parameters
    """
    f = h5py.File(filename,"w")
    
    for i in param_dict.keys():
        if(param_dict[i] is not None):
            f.attrs.create(i,param_dict[i])
        else:
            f.attrs.create(i,-1)
    f.attrs.create("validity",False)
    print filename, "initialized"
    f.close()

def init_hdf5_files(savepath,param_dict,matricesToLoad):
    svnversion = check_output("svnversion").replace("\n","")
    if not(matricesToLoad.has_key("A")):
        df=pandas.read_hdf(savepath+"matricesDataBase.h5","A")
        ind = len(df.index)
        filename = savepath+"turbu/A_r"+svnversion+"_"+str(ind)+".h5"
        create_file_attributes(filename,param_dict)
        updateDataBase(filename,savepath,"A")
        df=pandas.read_hdf(savepath+"matricesDataBase.h5","B")
        ind = len(df.index)
        filename = savepath+"turbu/B_r"+svnversion+"_"+str(ind)+".h5"
        create_file_attributes(filename,param_dict)
        updateDataBase(filename,savepath,"B")
        df=pandas.read_hdf(savepath+"matricesDataBase.h5","istx")
        ind = len(df.index)
        filename = savepath+"turbu/istx_r"+svnversion+"_"+str(ind)+".h5"
        create_file_attributes(filename,param_dict)
        updateDataBase(filename,savepath,"istx")
        df=pandas.read_hdf(savepath+"matricesDataBase.h5","isty")
        ind = len(df.index)
        filename = savepath+"turbu/isty_r"+svnversion+"_"+str(ind)+".h5"
        create_file_attributes(filename,param_dict)
        updateDataBase(filename,savepath,"isty")
        
    if not(matricesToLoad.has_key("pztok")):
        df=pandas.read_hdf(savepath+"matricesDataBase.h5","pztok")
        ind = len(df.index)
        filename = savepath+"mat/pztok_r"+svnversion+"_"+str(ind)+".h5"
        create_file_attributes(filename,param_dict)
        updateDataBase(filename,savepath,"pztok")
        df=pandas.read_hdf(savepath+"matricesDataBase.h5","pztnok")
        ind = len(df.index)
        filename = savepath+"mat/pztnok_r"+svnversion+"_"+str(ind)+".h5"
        create_file_attributes(filename,param_dict)
        updateDataBase(filename,savepath,"pztnok")
    if not(matricesToLoad.has_key("imat")):
        df=pandas.read_hdf(savepath+"matricesDataBase.h5","imat")
        ind = len(df.index)
        filename = savepath+"mat/imat_r"+svnversion+"_"+str(ind)+".h5"
        create_file_attributes(filename,param_dict)
        updateDataBase(filename,savepath,"imat")
        df=pandas.read_hdf(savepath+"matricesDataBase.h5","eigenv")
        ind = len(df.index)
        filename = savepath+"mat/eigenv_r"+svnversion+"_"+str(ind)+".h5"
        create_file_attributes(filename,param_dict)
        updateDataBase(filename,savepath,"eigenv")
        df=pandas.read_hdf(savepath+"matricesDataBase.h5","U")
        ind = len(df.index)
        filename = savepath+"mat/U_r"+svnversion+"_"+str(ind)+".h5"
        create_file_attributes(filename,param_dict)
        updateDataBase(filename,savepath,"U")
        
def initDataBase(savepath,param_dict):
    """ Initialize and create the database for all the saved matrices. This database
    will be placed on the top of the savepath and be named matricesDataBase.h5.
    
    :parameters:
        savepath : (str) : path to the data repertory
        param_dict : (dictionary) : parameters dictionary
    """
#    keys = ["revision","simulname","niter","ittime","zenithangle",
#            "pupdiam","tel_diam","cobs","r0","nscreens","frac","atm.alt",
#            "windspeed","winddir","L0","seeds","ntargets","target.xpos",
#            "target.ypos","target.Lambda","target.mag","target.dms_seen",
#            "nwfs","type_wfs","nxsub","npix","pixsize","fracsub","wfs.xpos",
#            "wfs.ypos","wfs.Lambda","gsmag","optthroughput","zerop","noise",
#            "atmos_seen","dms_seen","beamsize","fssize","fstop","gsalt",
#            "laserpower","lgsreturnperwatt","lltx","llty","openloop","proftype",
#            "pyr_ampl","pyr_loc","pyr_npts","pyrtype","ndms","type_dm","dm.alt",
#            "coupling","hyst","margin","nact","nkl","pupoffset","push4imat",
#            "dm.thresh","unitpervolt","ncentroiders","type_centro","nmax",
#            "centro.nwfs","sizex","sizey","centroider.thresh","type_fct",
#            "weights","width","ncontrollers","type_control","TTcond",
#            "cured_ndivs","delay","gain","maxcond","modopti","nactu","ndm",
#            "nmodes","nrec","nvalid","control.nwfs","path2file"] 
    keys = param_dict.keys()
    keys.append("path2file")
    keys.append("validity")
    df=pandas.DataFrame(columns=keys)
    store = pandas.HDFStore(savepath+"matricesDataBase.h5")
    store.put("A",df)
    store.put("B",df)
    store.put("istx",df)
    store.put("isty",df)
    store.put("eigenv",df)
    store.put("imat",df)
    store.put("pztok",df)
    store.put("pztnok",df)
    store.put("U",df)
    
    store.close()
    print "Matrices database created"    

def updateDataBase(h5file,savepath,matrix_type):
    """ Update the database adding a new row to the matrix_type database.
    
    :parameters:
        h5file : (str) : path to the new h5 file to add
        savepath : (str) : path to the data directory
        matrix_type : (str) : type of matrix to store ("A","B","istx","isty"
                                                         "istx","eigenv","imat","U"
                                                         "pztok" or "pztnok")
    """
    if(matrix_type =="A" or matrix_type =="B" or matrix_type =="istx"
        or matrix_type =="isty" or matrix_type =="eigenv" or matrix_type =="imat"
        or matrix_type =="U" or matrix_type =="pztok" or matrix_type =="pztnok"):
        f = h5py.File(h5file,"r")
        store = pandas.HDFStore(savepath+"matricesDataBase.h5")
        df = pandas.read_hdf(savepath+"matricesDataBase.h5",matrix_type)
        ind = len(df.index)
        for i in f.attrs.keys():
            df.loc[ind,i] = f.attrs[i]
        df.loc[ind,"path2file"] = h5file
        df.loc[ind,"validity"] = False
        store.put(matrix_type,df)
        store.close()
        f.close()
    else:
        raise ValueError("Wrong matrix_type specified. See documentation")
    
    
def save_hdf5(filename, dataname, data):
    """ save_hdf5(filename, dataname, data)
    Create a dataset in an existing hdf5 file filename and store data in it
    
    :param:
        filename: (str) : full path to the file
        dataname : (str) : name of the data (imat, cmat...)
        data : np.array : data to save
    """   
    f = h5py.File(filename,"r+")
    f.create_dataset(dataname, data=data)
    f.close()

def save_h5(filename, dataname, config,data):
    """ save_hdf5(filename, dataname, config, data)
    Create a hdf5 file and store data in it with full header from config parameters
    Usefull to backtrace data origins
    
    :param:
        filename: (str) : full path to the file
        dataname : (str) : name of the data (imat, cmat...)
        config : (module) : config parameters
        data : np.array : data to save
    """
    p_dict = params_dictionary(config)
    create_file_attributes(filename,p_dict)
    save_hdf5(filename, dataname, data)
    print filename,"has been written"
    
def checkMatricesDataBase(savepath,config,param_dict):
    """ Check in the database if the current config have been already run. If so,
    return a dictionary containing the matrices to load and their path. Matrices
    which don't appear in the dictionary will be computed, stored and added
    to the database during the simulation.
    If the database doesn't exist, this function creates it.
    
    :parameters:
        savepath : (str) : path to the data repertory
        config : (module) : simulation parameters
        param_dict : (dictionary) : parameters dictionary
    :return:
        matricesToLoad : (dictionary) : matrices that will be load and their path
    """
    
    matricesToLoad= {}
    if(os.path.exists(savepath+"matricesDataBase.h5")):
        
        checkTurbuParams(savepath,config,matricesToLoad)
        checkDmsParams(savepath,config,matricesToLoad)           
        checkControlParams(savepath,config,matricesToLoad)
          
    else:
        initDataBase(savepath,param_dict)
    init_hdf5_files(savepath,param_dict,matricesToLoad)
    return matricesToLoad       
    
def checkTurbuParams(savepath,config,matricesToLoad):
    """ Compare the current turbulence parameters to the database. If similar parameters
    are found, the matricesToLoad dictionary is completed.
    Since all the turbulence matrices are computed together, we only check the parameters
    for the A matrix : if we load A, we load B, istx and isty too.
    
    :parameters:
        config : (module) : simulation parameters
        matricesToLoad : (dictionary) :  matrices that will be load and their path
    """
    dataBase = pandas.read_hdf(savepath+"matricesDataBase.h5","A")
    for i in dataBase.index:
        if(dataBase.loc[i,"validity"]):
            load_turbu = (dataBase.loc[i,"revision"] == check_output("svnversion").replace("\n",""))
            load_turbu &= ((dataBase.loc[i,"L0"] == config.p_atmos.L0).all())
            load_turbu &= ((dataBase.loc[i,"atm.alt"] == config.p_atmos.alt).all())
            load_turbu &= ((dataBase.loc[i,"tel_diam"] == config.p_tel.diam).all())
            load_turbu &= ((dataBase.loc[i,"cobs"] == config.p_tel.cobs).all())
            load_turbu &= ((dataBase.loc[i,"pupdiam"] == config.p_geom.pupdiam).all())
            load_turbu &= ((dataBase.loc[i,"zenithangle"] == config.p_geom.zenithangle).all())
            load_turbu &= ((dataBase.loc[i,"target.xpos"] == config.p_target.xpos).all())
            load_turbu &= ((dataBase.loc[i,"target.ypos"] == config.p_target.ypos).all())
            load_turbu &= ((dataBase.loc[i,"wfs.xpos"] == [wfs.xpos for wfs in config.p_wfss]).all())
            load_turbu &= ((dataBase.loc[i,"wfs.ypos"] == [wfs.ypos for wfs in config.p_wfss]).all())
            
            if(load_turbu):
                matricesToLoad["index_turbu"] = i
                matricesToLoad["A"] = dataBase.loc[i,"path2file"]
                dataBase = pandas.read_hdf(savepath+"matricesDataBase.h5","B")
                matricesToLoad["B"] = dataBase.loc[i,"path2file"]
                dataBase = pandas.read_hdf(savepath+"matricesDataBase.h5","istx")
                matricesToLoad["istx"] = dataBase.loc[i,"path2file"]
                dataBase = pandas.read_hdf(savepath+"matricesDataBase.h5","isty")
                matricesToLoad["isty"] = dataBase.loc[i,"path2file"]
                return

def checkControlParams(savepath,config,matricesToLoad):
    """ Compare the current controller parameters to the database. If similar parameters
    are found, matricesToLoad dictionary is completed.
    Since all the controller matrices are computed together, we only check the parameters
    for the imat matrix : if we load imat, we load eigenv and U too.
    
    :parameters:
        config : (module) : simulation parameters
        matricesToLoad : (dictionary) :  matrices that will be load and their path
    """
    dataBase = pandas.read_hdf(savepath+"matricesDataBase.h5","imat")

    for i in dataBase.index:
        if(dataBase.loc[i,"validity"]):
            load_control = (dataBase.loc[i,"revision"] == check_output("svnversion").replace("\n",""))
            load_control &= ((dataBase.loc[i,"tel_diam"] == config.p_tel.diam).all())
            load_control &= ((dataBase.loc[i,"cobs"] == config.p_tel.cobs).all())
            load_control &= ((dataBase.loc[i,"pupdiam"] == config.p_geom.pupdiam).all())
            # Check WFS params
            load_control &= (dataBase.loc[i,"nwfs"] == len(config.p_wfss))
            load_control &= ((dataBase.loc[i,"type_wfs"] == [wfs.type_wfs for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"type_wfs"] == [wfs.type_wfs for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"nxsub"] == [wfs.nxsub for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"npix"] == [wfs.npix for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"pixsize"] == [wfs.pixsize for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"fracsub"] == [wfs.fracsub for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"wfs.xpos"] == [wfs.xpos for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"wfs.ypos"] == [wfs.ypos for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"wfs.Lambda"] == [wfs.Lambda for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"gsmag"] == [wfs.gsmag for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"optthroughput"] == [wfs.optthroughput for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"zerop"] == [wfs.zerop for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"noise"] == [wfs.noise for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"atmos_seen"] == [wfs.atmos_seen for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"dms_seen"] == [wfs.dms_seen if(wfs.dms_seen is not None) else np.arange(len(config.p_dms),dtype=np.int32) for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"beamsize"] == [wfs.beamsize for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"fssize"] == [wfs.fssize for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"fstop"] == [wfs.fstop if(wfs.fstop) else "" for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"gsalt"] == [wfs.gsalt for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"laserpower"] == [wfs.laserpower for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"lgsreturnperwatt"] == [wfs.lgsreturnperwatt for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"lltx"] == [wfs.lltx for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"llty"] == [wfs.llty for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"openloop"] == [wfs.openloop for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"proftype"] == [wfs.proftype if(wfs.proftype) else "" for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"pyr_ampl"] == [wfs.pyr_ampl for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"pyr_loc"] == [wfs.pyr_loc if(wfs.pyr_loc) else ""  for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"pyr_npts"] == [wfs.pyr_npts for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"pyrtype"] == [wfs.pyrtype if(wfs.pyrtype) else "" for wfs in config.p_wfss]).all())
    # Dms params
            load_control &= (dataBase.loc[i,"ndms"] == len(config.p_dms))
            load_control &= ((dataBase.loc[i,"type_dm"] == [dm.type_dm for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"dm.alt"] == [dm.alt for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"coupling"] == [dm.coupling for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"hyst"] == [dm.hyst for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"margin"] == [dm.margin for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"nact"] == [dm.nact for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"nkl"] == [dm.type_dm for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"push4imat"] == [dm.push4imat for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"dm.thresh"] == [dm.thresh for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"unitpervolt"] == [dm.unitpervolt for dm in config.p_dms]).all())
        
        # Centroider params
            load_control &= (dataBase.loc[i,"ncentroiders"] == len(config.p_centroiders))
            load_control &= ((dataBase.loc[i,"type_centro"] == [c.type_centro for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"nmax"] == [c.nmax for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"centro.nwfs"] == [c.nwfs for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"sizex"] == [c.sizex for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"sizey"] == [c.sizey for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"centroider.thresh"] == [c.thresh for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"type_fct"] == [c.type_fct if(c.type_fct) else "" for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"weights"] == [c.weights if(c.weights) else 0 for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"width"] == [c.width for c in config.p_centroiders]).all())
        
        # Controller params
            load_control &= (dataBase.loc[i,"ncontrollers"] == len(config.p_controllers))
            load_control &= ((dataBase.loc[i,"type_control"] == [c.type_control for c in config.p_controllers]).all())
            load_control &= ((dataBase.loc[i,"cured_ndivs"] == [c.cured_ndivs for c in config.p_controllers]).all())
            load_control &= ((dataBase.loc[i,"ndm"] == [c.ndm for c in config.p_controllers]).all())
            load_control &= ((dataBase.loc[i,"nmodes"] == [c.nmodes for c in config.p_controllers]).all())        
            load_control &= ((dataBase.loc[i,"control.nwfs"] == [c.nwfs for c in config.p_controllers]).all())
            
            if(load_control):
                matricesToLoad["index_control"] = i
                matricesToLoad["imat"] = dataBase.loc[i,"path2file"]
                dataBase = pandas.read_hdf(savepath+"matricesDataBase.h5","eigenv")
                matricesToLoad["eigenv"] = dataBase.loc[i,"path2file"]
                dataBase = pandas.read_hdf(savepath+"matricesDataBase.h5","U")
                matricesToLoad["U"] = dataBase.loc[i,"path2file"]
                return
        
def checkDmsParams(savepath,config,matricesToLoad):
    """ Compare the current controller parameters to the database. If similar parameters
    are found, matricesToLoad dictionary is completed.
    Since all the dms matrices are computed together, we only check the parameters
    for the pztok matrix : if we load pztok, we load pztnok too.
    
    :parameters:
        config : (module) : simulation parameters
        matricesToLoad : (dictionary) :  matrices that will be load and their path
    """
    dataBase = pandas.read_hdf(savepath+"matricesDataBase.h5","pztok")
    for i in dataBase.index:
        if(dataBase.loc[i,"validity"]):
            load_control = (dataBase.loc[i,"revision"] == check_output("svnversion").replace("\n",""))
            load_control &= ((dataBase.loc[i,"tel_diam"] == config.p_tel.diam).all())
            load_control &= ((dataBase.loc[i,"cobs"] == config.p_tel.cobs).all())
            load_control &= ((dataBase.loc[i,"pupdiam"] == config.p_geom.pupdiam).all())
            # Check WFS params
            load_control &= (dataBase.loc[i,"nwfs"] == len(config.p_wfss))
            load_control &= ((dataBase.loc[i,"type_wfs"] == [wfs.type_wfs for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"type_wfs"] == [wfs.type_wfs for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"nxsub"] == [wfs.nxsub for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"npix"] == [wfs.npix for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"pixsize"] == [wfs.pixsize for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"fracsub"] == [wfs.fracsub for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"wfs.xpos"] == [wfs.xpos for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"wfs.ypos"] == [wfs.ypos for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"wfs.Lambda"] == [wfs.Lambda for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"gsmag"] == [wfs.gsmag for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"optthroughput"] == [wfs.optthroughput for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"zerop"] == [wfs.zerop for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"noise"] == [wfs.noise for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"atmos_seen"] == [wfs.atmos_seen for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"dms_seen"] == [wfs.dms_seen if(wfs.dms_seen is not None) else np.arange(len(config.p_dms),dtype=np.int32) for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"beamsize"] == [wfs.beamsize for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"fssize"] == [wfs.fssize for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"fstop"] == [wfs.fstop if(wfs.fstop) else "" for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"gsalt"] == [wfs.gsalt for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"laserpower"] == [wfs.laserpower for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"lgsreturnperwatt"] == [wfs.lgsreturnperwatt for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"lltx"] == [wfs.lltx for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"llty"] == [wfs.llty for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"openloop"] == [wfs.openloop for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"proftype"] == [wfs.proftype if(wfs.proftype) else "" for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"pyr_ampl"] == [wfs.pyr_ampl for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"pyr_loc"] == [wfs.pyr_loc if(wfs.pyr_loc) else ""  for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"pyr_npts"] == [wfs.pyr_npts for wfs in config.p_wfss]).all())
            load_control &= ((dataBase.loc[i,"pyrtype"] == [wfs.pyrtype if(wfs.pyrtype) else "" for wfs in config.p_wfss]).all())
    # Dms params
            load_control &= (dataBase.loc[i,"ndms"] == len(config.p_dms))
            load_control &= ((dataBase.loc[i,"type_dm"] == [dm.type_dm for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"dm.alt"] == [dm.alt for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"coupling"] == [dm.coupling for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"hyst"] == [dm.hyst for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"margin"] == [dm.margin for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"nact"] == [dm.nact for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"nkl"] == [dm.type_dm for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"push4imat"] == [dm.push4imat for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"dm.thresh"] == [dm.thresh for dm in config.p_dms]).all())
            load_control &= ((dataBase.loc[i,"unitpervolt"] == [dm.unitpervolt for dm in config.p_dms]).all())
        
        # Centroider params
            load_control &= (dataBase.loc[i,"ncentroiders"] == len(config.p_centroiders))
            load_control &= ((dataBase.loc[i,"type_centro"] == [c.type_centro for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"nmax"] == [c.nmax for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"centro.nwfs"] == [c.nwfs for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"sizex"] == [c.sizex for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"sizey"] == [c.sizey for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"centroider.thresh"] == [c.thresh for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"type_fct"] == [c.type_fct if(c.type_fct) else "" for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"weights"] == [c.weights if(c.weights) else 0 for c in config.p_centroiders]).all())
            load_control &= ((dataBase.loc[i,"width"] == [c.width for c in config.p_centroiders]).all())
        
        # Controller params
            load_control &= (dataBase.loc[i,"ncontrollers"] == len(config.p_controllers))
            load_control &= ((dataBase.loc[i,"type_control"] == [c.type_control for c in config.p_controllers]).all())
            load_control &= ((dataBase.loc[i,"cured_ndivs"] == [c.cured_ndivs for c in config.p_controllers]).all())
            load_control &= ((dataBase.loc[i,"ndm"] == [c.ndm for c in config.p_controllers]).all())
            load_control &= ((dataBase.loc[i,"nmodes"] == [c.nmodes for c in config.p_controllers]).all())        
            load_control &= ((dataBase.loc[i,"control.nwfs"] == [c.nwfs for c in config.p_controllers]).all())
            
            if(load_control):
                matricesToLoad["index_dms"] = i
                dataBase = pandas.read_hdf(savepath+"matricesDataBase.h5","pztnok")
                matricesToLoad["pztnok"] = dataBase.loc[i,"path2file"]
                dataBase = pandas.read_hdf(savepath+"matricesDataBase.h5","pztok")
                matricesToLoad["pztok"] = dataBase.loc[i,"path2file"]             
                return

def validDataBase(savepath,matricesToLoad):
    store = pandas.HDFStore(savepath+"matricesDataBase.h5")
    if not(matricesToLoad.has_key("A")):
        validInStore(store,savepath,"A")
        validInStore(store,savepath,"B")
        validInStore(store,savepath,"istx")
        validInStore(store,savepath,"isty")
    if not(matricesToLoad.has_key("pztok")):
        validInStore(store,savepath,"pztok")
        validInStore(store,savepath,"pztnok")
    if not(matricesToLoad.has_key("imat")):
        validInStore(store,savepath,"imat")
    if not(matricesToLoad.has_key("eigenv")):
        validInStore(store,savepath,"eigenv")
        validInStore(store,savepath,"U")
    store.close()

def validFile(filename):
    f=h5py.File(filename,"r+")
    f.attrs["validity"] = True
    f.close()

def validInStore(store,savepath,matricetype):
    df = pandas.read_hdf(savepath+"matricesDataBase.h5",matricetype)
    ind = len(df.index) - 1
    df.loc[ind,"validity"] = True
    store[matricetype] = df
    validFile(df.loc[ind,"path2file"])
    
def configFromH5(filename,config):
    import shesha as ao
    
    f=h5py.File(filename,"r")
    
    config.simul_name = str(f.attrs.get("simulname"))
    #Loop
    config.p_loop.set_niter(f.attrs.get("niter"))
    config.p_loop.set_ittime(f.attrs.get("ittime"))
    
    #geom
    config.p_geom.set_zenithangle(f.attrs.get("zenithangle"))
    config.p_geom.set_pupdiam(f.attrs.get("pupdiam"))
    
    # Tel
    config.p_tel.set_diam(f.attrs.get("tel_diam"))
    config.p_tel.set_cobs(f.attrs.get("cobs"))
    
    # Atmos
    config.p_atmos.set_r0(f.attrs.get("r0"))
    config.p_atmos.set_nscreens(f.attrs.get("nscreens"))
    config.p_atmos.set_frac(f.attrs.get("frac"))
    config.p_atmos.set_alt(f.attrs.get("atm.alt"))
    config.p_atmos.set_windspeed(f.attrs.get("windspeed"))
    config.p_atmos.set_winddir(f.attrs.get("winddir"))
    config.p_atmos.set_L0(f.attrs.get("L0"))
    config.p_atmos.set_seeds(f.attrs.get("seeds"))
    
    # Target
    config.p_target.set_nTargets(f.attrs.get("ntargets"))
    config.p_target.set_xpos(f.attrs.get("target.xpos"))
    config.p_target.set_ypos(f.attrs.get("target.ypos"))
    config.p_target.set_Lambda(f.attrs.get("target.Lambda"))
    config.p_target.set_mag(f.attrs.get("target.mag"))
    if(f.attrs.get("target.dms_seen") > -1):
        config.p_target.set_dms_seen(f.attrs.get("target.dms_seen"))
    
    # WFS
    config.p_wfss=[]
    for i in range(f.attrs.get("nwfs")):
        config.p_wfss.append(ao.Param_wfs())
        config.p_wfss[i].set_type(str(f.attrs.get("type_wfs")[i]))
        config.p_wfss[i].set_nxsub(f.attrs.get("nxsub")[i])
        config.p_wfss[i].set_npix(f.attrs.get("npix")[i])
        config.p_wfss[i].set_pixsize(f.attrs.get("pixsize")[i])
        config.p_wfss[i].set_fracsub(f.attrs.get("fracsub")[i])
        config.p_wfss[i].set_xpos(f.attrs.get("wfs.xpos")[i])
        config.p_wfss[i].set_ypos(f.attrs.get("wfs.ypos")[i])
        config.p_wfss[i].set_Lambda(f.attrs.get("wfs.Lambda")[i])
        config.p_wfss[i].set_gsmag(f.attrs.get("gsmag")[i])
        config.p_wfss[i].set_optthroughput(f.attrs.get("optthroughput")[i])
        config.p_wfss[i].set_zerop(f.attrs.get("zerop")[i])
        config.p_wfss[i].set_noise(f.attrs.get("noise")[i])
        config.p_wfss[i].set_atmos_seen(f.attrs.get("atmos_seen")[i])
        config.p_wfss[i].set_fstop(str(f.attrs.get("fstop")[i]))
        config.p_wfss[i].set_pyr_npts(f.attrs.get("pyr_npts")[i])
        config.p_wfss[i].set_pyr_ampl(f.attrs.get("pyr_ampl")[i])
        config.p_wfss[i].set_pyrtype(str(f.attrs.get("pyrtype")[i]))
        config.p_wfss[i].set_pyr_loc(str(f.attrs.get("pyr_loc")[i]))
        config.p_wfss[i].set_fssize(f.attrs.get("fssize")[i])
        if((f.attrs.get("dms_seen")[i] > -1).all()):
            config.p_wfss[i].set_dms_seen(f.attrs.get("dms_seen")[i])
        
        # LGS
        config.p_wfss[i].set_gsalt(f.attrs.get("gsalt")[i])
        config.p_wfss[i].set_lltx(f.attrs.get("lltx")[i])
        config.p_wfss[i].set_llty(f.attrs.get("llty")[i])
        config.p_wfss[i].set_laserpower(f.attrs.get("laserpower")[i])
        config.p_wfss[i].set_lgsreturnperwatt(f.attrs.get("lgsreturnperwatt")[i])
        config.p_wfss[i].set_proftype(str(f.attrs.get("proftype")[i]))
        config.p_wfss[i].set_beamsize(f.attrs.get("beamsize")[i])
    
    # DMs
    config.p_dms=[]
    if(f.attrs.get("ndms")):
        for i in range(f.attrs.get("ndms")):
            config.p_dms.append(ao.Param_dm())
            config.p_dms[i].set_type(str(f.attrs.get("type_dm")[i]))
            config.p_dms[i].set_nact(f.attrs.get("nact")[i])
            config.p_dms[i].set_alt(f.attrs.get("dm.alt")[i])
            config.p_dms[i].set_thresh(f.attrs.get("dm.thresh")[i])
            config.p_dms[i].set_coupling(f.attrs.get("coupling")[i])
            config.p_dms[i].set_unitpervolt(f.attrs.get("unitpervolt")[i])
            config.p_dms[i].set_push4imat(f.attrs.get("push4imat")[i])
    
    # Centroiders
    config.p_centroiders=[]
    if(f.attrs.get("ncentroiders")):
        for i in range(f.attrs.get("ncentroiders")):
            config.p_centroiders.append(ao.Param_centroider())
            config.p_centroiders[i].set_nwfs(f.attrs.get("centro.nwfs")[i])
            config.p_centroiders[i].set_type(str(f.attrs.get("type_centro")[i]))
            config.p_centroiders[i].set_type_fct(str(f.attrs.get("type_fct")[i]))
            config.p_centroiders[i].set_nmax(f.attrs.get("nmax")[i])
            config.p_centroiders[i].set_thresh(f.attrs.get("centroider.thresh")[i])
            if(f.attrs.get("weights")[i]):
                config.p_centroiders[i].set_weights(f.attrs.get("weights")[i])
            config.p_centroiders[i].set_width(f.attrs.get("width")[i])
        config.p_rtc.set_centroiders(config.p_centroiders)
    
    # Controllers
    config.p_controllers=[]
    if(f.attrs.get("ncontrollers")):
        for i in range(f.attrs.get("ncontrollers")):
            config.p_controllers.append(ao.Param_controller())
            config.p_controllers[i].set_type(str(f.attrs.get("type_control")[i]))
            config.p_controllers[i].set_nwfs(f.attrs.get("control.nwfs")[i])
            config.p_controllers[i].set_ndm(f.attrs.get("ndm")[i])
            config.p_controllers[i].set_maxcond(f.attrs.get("maxcond")[i])
            config.p_controllers[i].set_delay(f.attrs.get("delay")[i])
            config.p_controllers[i].set_gain(f.attrs.get("gain")[i])
            config.p_controllers[i].set_modopti(f.attrs.get("modopti")[i])
            config.p_controllers[i].set_nrec(f.attrs.get("nrec")[i])
            config.p_controllers[i].set_nmodes(f.attrs.get("nmodes")[i])
            config.p_controllers[i].set_gmin(f.attrs.get("gmin")[i])
            config.p_controllers[i].set_gmax(f.attrs.get("gmax")[i])
            config.p_controllers[i].set_ngain(f.attrs.get("ngain")[i])
            config.p_controllers[i].set_TTcond(f.attrs.get("TTcond")[i])
            config.p_controllers[i].set_cured_ndivs(f.attrs.get("cured_ndivs")[i])
        config.p_rtc.set_controllers(config.p_controllers)
    
    config.p_rtc.set_nwfs(f.attrs.get("nwfs"))
    
    print "Parameters have been read from ",filename, "header"
        
        
    
    