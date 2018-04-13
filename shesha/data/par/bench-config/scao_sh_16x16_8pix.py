import shesha_config as conf


class Param_camera:

    def __init__(self) -> None:
        self.camAddr = ""
        self.width = 0
        self.height = 0
        self.offset_w = 0
        self.offset_h = 0
        self.expo_usec = 0
        self.framerate = 0


p_cams = [Param_camera()]
p_cams[0].camAddr = ""
p_cams[0].width = 128
p_cams[0].height = 128
p_cams[0].offset_w = 0
p_cams[0].offset_h = 0
p_cams[0].expo_usec = 1000
p_cams[0].framerate = 100

p_wfss = [conf.Param_wfs()]
p_wfss[0].set_type("sh")
p_wfss[0].set_nxsub(16)
p_wfss[0].set_npix(8)
p_nvalid = None  # [X0, X1, ..., XN, Y0, Y1, ..., YN]

p_dms = [conf.Param_dm()]
p_dms[0].set_type("pzt")
p_dms[0].set_alt(0.)
p_dms[0].set_nact(4096)

# centroiders0
p_centroiders = [conf.Param_centroider()]
p_centroiders[0].set_nwfs(0)
p_centroiders[0].set_type("cog")
# p_centroiders[0].set_type("corr")
# p_centroiders[0].set_type_fct("model")

# controllers
p_controllers = [conf.Param_controller()]
p_controllers[0].set_type("ls")
p_controllers[0].set_nwfs([0])
p_controllers[0].set_ndm([0, 1])
p_controllers[0].set_maxcond(100.)
p_controllers[0].set_delay(1)
p_controllers[0].set_gain(0.4)
