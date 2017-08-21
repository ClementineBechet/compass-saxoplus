import sys
import os
from naga import naga_context
import shesha_init as init
import shesha_constants as scons
import time


class Simulator:

    def __init__(self, filepath=None):
        """
        TODO: docstring
        """
        self.is_init = False
        self.loaded = False
        self.config = None

        if filepath is not None:
            self.load_from_file(filepath)

    def __str__(self):
        """
        TODO: docstring
        """
        if self.is_init:
            print("====================")
            print("objects initialzed on GPU:")
            print("--------------------------------------------------------")

            if self.atm is not None:
                print(self.atm)
            if self.wfs is not None:
                print(self.wfs)
            if self.dms is not None:
                print(self.dms)
            if self.tar is not None:
                print(self.tar)
            if self.rtc is not None:
                print(self.rtc)

    def load_from_file(self, filepath: str):
        """
        TODO: docstring
        """
        self.loaded = False
        self.is_init = False
        filename = filepath.split('/')[-1]
        if (filepath.split('.')[-1] != "py"):
            raise ValueError("Config file must be .py")

        pathfile = filepath.split(filename)[0]
        if (pathfile not in sys.path):
            sys.path.insert(0, pathfile)

        if self.config is not None:
            name = self.config.__name__
            print("Removing previous config")
            self.config = None
            try:
                del sys.modules[name]
            except:
                pass

        print(("loading ", filename.split(".py")[0]))
        self.config = __import__(filename.split(".py")[0])
        # exec("import %s as wao_config" % filename.split(".py")[0])
        sys.path.remove(pathfile)

        self.loaded = True

    def init_sim(self):
        """
        TODO: docstring
        """
        if not self.loaded:
            raise ValueError("Config must be loaded before call to init_sim")
        c = naga_context(devices=self.config.p_loop.devices)

        if self.config.p_tel is None or self.config.p_geom is None:
            raise ValueError(
                    "Telescope geometry must be defined (p_geom and p_tel)")

        if self.config.p_atmos is not None:
            r0 = self.config.p_atmos.r0
        if self.config.p_loop is not None:
            ittime = self.config.p_loop.ittime

        print("->tel")
        self.tel = init.tel_init(
                c, self.config.p_geom, self.config.p_tel, r0, ittime,
                self.config.p_wfss)

        if self.config.p_atmos is not None:
            #   atmos
            print("->atmos")
            self.atm = init.atmos_init(
                    c, self.config.p_atmos, self.config.p_tel,
                    self.config.p_geom, ittime)
        else:
            self.atm = None

        if self.config.p_dms is not None:
            #   dm
            print("->dm")
            self.dms = init.dm_init(
                    c, self.config.p_dms, self.config.p_tel,
                    self.config.p_geom, self.config.p_wfss)
        else:
            self.dms = None

        if self.config.p_target is not None:
            print("->target")
            self.tar = init.target_init(
                    c, self.tel, self.config.p_target, self.config.p_atmos,
                    self.config.p_tel, self.config.p_geom, self.config.p_dms)
        else:
            self.tar = None

        if self.config.p_wfss is not None:
            print("->wfs")
            self.wfs = init.wfs_init(
                    c, self.tel, self.config.p_wfss, self.config.p_tel,
                    self.config.p_geom, self.config.p_dms, self.config.p_atmos)
        else:
            self.wfs = None

        if self.config.p_controllers is not None or self.config.p_centroiders is not None:
            print("->rtc")
            #   rtc
            self.rtc = init.rtc_init(
                    c, self.tel, self.wfs, self.dms, self.atm,
                    self.config.p_wfss, self.config.p_tel, self.config.p_geom,
                    self.config.p_atmos, ittime, self.config.p_centroiders,
                    self.config.p_controllers, self.config.p_dms)
        else:
            self.rtc = None

        self.is_init = True

    def next(self):
        """
        TODO: docstring
        """
        self.atm.move_atmos()

        if (
                self.config.p_controllers[0].type_control ==
                scons.ControllerType.GEO):
            for t in range(self.config.p_target.ntargets):
                self.tar.raytrace(t, b"atmos", self.tel, self.atm)
                self.rtc.do_control_geo(0, self.dms, self.tar, 0)
                self.rtc.apply_control(0, self.dms)
                self.tar.raytrace(t, b"dm", self.tel, dms=self.dms)
        else:
            for t in range(self.config.p_target.ntargets):
                self.tar.raytrace(t, b"all", self.tel, self.atm, self.dms)
            for w in range(len(self.config.p_wfss)):
                self.wfs.raytrace(w, b"all", self.tel, self.atm, self.dms)
                self.wfs.comp_img(w)

            self.rtc.do_centroids(0)
            self.rtc.do_control(0)

            self.rtc.apply_control(0, self.dms)

    def loop(self, n=1, monitoring_freq=100):
        """
        TODO: docstring
        """
        print("----------------------------------------------------")
        print("iter# | S.E. SR | L.E. SR | Est. Rem. | framerate")
        print("----------------------------------------------------")
        t0 = time.time()
        for i in range(n):
            self.next()
            if ((i + 1) % monitoring_freq == 0):
                strehltmp = self.tar.get_strehl(0)
                print(i + 1, "\t", strehltmp[0], "\t", strehltmp[1])
        t1 = time.time()
        print(
                " loop execution time:", t1 - t0, "  (", n, "iterations), ",
                (t1 - t0) / n, "(mean)  ", n / (t1 - t0), "Hz")
