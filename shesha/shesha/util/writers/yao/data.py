from astropy.io import fits
import numpy as np

def get_yao_subap_pos_single(sup, wfs_id):
    """Return the coordinates of the valid subapertures of a given WFS

    this coordinates are given in meters and centered

    Parameters:
        sup : (compassSupervisor) : supervisor

        wfs_id : (int) : index of the WFS

    Return :
        valid_X : (np.ndarray[ndim=1, dtype=np.float64]) : subapertures positions along axis x

        valid_Y : (np.ndarray[ndim=1, dtype=np.float64]) : subapertures positions along axis y
    """

    config = sup.config
    wfs = config.p_wfss[wfs_id]
    geom = config.p_geom
    total = geom.pupdiam/wfs.nxsub*(wfs.nxsub-1)
    valid_X = wfs._validpuppixx-2
    valid_Y = wfs._validpuppixy-2
    toMeter = (config.p_tel.diam/wfs.nxsub/wfs._pdiam)
    valid_X = (valid_X-total/2)*toMeter
    valid_Y = (valid_Y-total/2)*toMeter
    return valid_X, valid_Y


def get_yao_subap_pos(sup, *, n_wfs=-1):

    """return the number of valid subapertures for all WFS as well as their coordinates
    
    the coordinates are given in meters and centered

    Parameters:
        sup : (compasSSupervisor) : supervisor

        n_wfs : (int) : number of wfs

    Return:
        n_valid : (np.ndarray[ndim=1, dtype=np.int32]) : number of valid subapertures per wfs

        valid_subap : (np.ndarray[ndim=1, dtype=np.float64]) : subapertures coordinates
    """

    config=sup.config
    if(n_wfs < 0):
        n_wfs = len(config.p_wfss)

    n_valid = np.array([w._nvalid for w in config.p_wfss[:n_wfs]])
    valid_subap = np.zeros((2,np.sum(n_valid)))
    ind = 0
    for w in range(n_wfs):
        valid_subap[0,ind:ind+n_valid[w]], \
        valid_subap[1,ind:ind+n_valid[w]] = get_yao_subap_pos_single(sup, w)
        ind += n_valid[w]

    return n_valid.astype(np.int32), valid_subap.astype(np.float64)


def get_yao_actuPos_single(sup, dm_id):
    """return the coordinates of a given DM actuators for YAO

    Parameters:
        sup : (compasSSupervisor) : supervisor

        dm_id : (int) : index of the DM

    Return:
        xpos : (np.ndarray[ndim=1, dtype=np.float32]) : actuators positions along axis x

        ypos : (np.ndarray[ndim=1, dtype=np.float32]) : actuators positions along axis y
    """

    dm=sup.config.p_dms[dm_id]
    return dm._xpos+1, dm._ypos+1


def get_yao_actu_pos(sup):
    """return the coordinates of all  DM actuators for YAO

    Parameters:
        sup : (compasSSupervisor) : supervisor

    Return:
        n_actu : (np.ndarray[ndim=1, dtype=np.int32]) : number of valid actuators per dm

        valid_subap : (np.ndarray[ndim=1, dtype=np.float64]) : actuators coordinates

    """
    config = sup.config
    n_actu = np.array([dm._ntotact for dm in config.p_dms])
    actu_pos = np.zeros((2,np.sum(n_actu)))
    ind=0
    for dm in range(len(config.p_dms)):
        if(sup.config.p_dms[dm].type !="tt"):
            actu_pos[0,ind:ind+n_actu[dm]],actu_pos[1,ind:ind+n_actu[dm]]=get_yao_actuPos_single(sup,dm)
        ind+=n_actu[dm]
    return n_actu.astype(np.int32), actu_pos.astype(np.float64)


def write_data(file_name, sup, *, n_wfs=-1 ,controller_id=0 ,
               compose_type="controller"):
    """ Write data for yao compatibility

    write into a single fits:
        * number of valide subapertures
        * number of actuators
        * subapertures position (2-dim array x,y) in meters centered
        * actuator position (2-dim array x,y) in pixels starting from 0
        * interaction matrix (2*nSubap , nactu)
        * command matrix (nacy , 2*nSubap)

    Parameters:
        file_name : (str) : data file name

        sup : (compasSSupervisor) : supervisor

        n_wfs : (int) : number of wfs passed to yao

        controller_id : (int) : index of the controller passed to yao

        compose_type : (str) : possibility to specify split tomography case ("controller" or "splitTomo")
    """

    print("writing data to"+file_name)
    hdu = fits.PrimaryHDU(np.zeros(1,dtype=np.int32))
    config = sup.config
    n_actu = config.p_controllers[controller_id].nactu

    #get nb of subap and their position
    n_valid,subap_pos=get_yao_subap_pos(sup,n_wfs=n_wfs)
    n_valid_total=np.sum(n_valid)
    hdu.header["NTOTSUB"]=n_valid_total
    hdu_nsubap=fits.ImageHDU(n_valid,name="NSUBAP")
    hdu_subap_pos=fits.ImageHDU(subap_pos,name="SUBAPPOS")

    #get nb of actu and their position
    n_actu,actu_pos=get_yao_actu_pos(sup)
    n_actu_total=np.sum(n_actu)
    hdu.header["NTOTACTU"]=n_actu_total
    hdu_nactu=fits.ImageHDU(n_actu,name="NACTU")
    hdu_actu_pos=fits.ImageHDU(actu_pos,name="ACTUPOS")

    #IMAT
    imat=compose_imat(sup, compose_type=compose_type,
                      controller_id=controller_id)
    hdu_imat=fits.ImageHDU(imat,name="IMAT")

    #CMAT
    hdu_cmat=fits.ImageHDU(sup.rtc.get_command_matrix(controller_id),
                           name="CMAT")

    print("\t* number of subaperture per WFS")
    print("\t* subapertures position")
    print("\t* number of actuator per DM")
    print("\t* actuators position")
    print("\t* Imat")
    print("\t* Cmat")

    hdul=fits.HDUList([hdu, hdu_nsubap, hdu_subap_pos, hdu_nactu, hdu_actu_pos,
                       hdu_imat, hdu_cmat])
    hdul.writeto(file_name, overwrite=1)


def compose_imat(sup, *, compose_type="controller", controller_id=0):
    """ Return an interaction matrix

    return either the specified controller interaction matrix (if compose_type="controller")
    or an imat composed of all controller interaction matrices (if compose_type="splitTomo")

    Parameters:
        sup : (compasSSupervisor) : supervisor

        compose_type : (str) : (optional), default "controller" possibility to specify split tomography case ("controller" or "splitTomo")

        controller_id : (int) : (optional), default 0 controller index

    Return:
        imat : (np.ndarray[ndim=1, dtype=np.float32]) : interaction matrix
    """
    if(compose_type=="controller"):
        return sup.rtc.get_interaction_matrix(controller_id)
    elif(compose_type=="splitTomo"):
        n_actu = 0
        n_meas = 0
        for c in range(len(sup.config.p_controllers)):
            imShape = sup.rtc.get_interaction_matrix(c).shape
            n_meas += imShape[0]
            n_actu += imShape[1]
        imat=np.zeros((n_meas, n_actu))
        n_meas = 0
        n_actu = 0
        for c in range(len(sup.config.p_controllers)):
            im = sup.rtc.get_interaction_matrix(c)
            imat[n_meas:n_meas+im.shape[0],n_actu:n_actu+im.shape[1]]=np.copy(im)
            n_meas += im.shape[0]
            n_actu += im.shape[1]
        return imat

    else:
        print("Unknown composition type")
