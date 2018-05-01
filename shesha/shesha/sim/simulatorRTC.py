"""
Class SimulatorRTC: COMPASS simulation linked to real RTC with Octopus
"""
import os
import sys

import numpy as np

import Octopus
from shesha.constants import CentroiderType, WFSType

from .simulator import Iterable, Simulator, init, load_config_from_file


class SimulatorRTC(Simulator):
    """
        Class SimulatorRTC: COMPASS simulation linked to real RTC with Octopus
    """

    def __init__(self, filepath: str=None) -> None:
        """
        Initializes a Simulator instance

        :parameters:
            filepath: (str): (optional) path to the parameters file
            rtcfilepath: (str): (optional) path to the rtc parameters file
            use_DB: (bool): (optional) flag to use dataBase system
        """
        Simulator.__init__(self)
        self.rtcconf = lambda x: None

        if filepath is not None:
            self.load_from_file(filepath)

    def load_from_file(self, filepath: str) -> None:
        """
        Load the parameters from the parameters file

        :parameters:
            filepath: (str): path to the parameters file

        """
        if "KRAKEN_ROOT" in os.environ:
            kraken_pathfile = "{}/KrakenConf/data".format(os.environ["KRAKEN_ROOT"])
            if (kraken_pathfile not in sys.path):
                sys.path.insert(0, kraken_pathfile)

        load_config_from_file(self.rtcconf, filepath)
        Simulator.load_from_file(self, self.rtcconf.config.p_sim)

    def init_sim(self) -> None:
        Simulator.init_sim(self)

        wfsNb = len(self.rtcconf.config.p_wfss)
        if wfsNb > 1:
            raise RuntimeError("multi WFS not supported")
        p_wfs = self.rtcconf.config.p_wfss[0]

        nact = self.rtcconf.config.p_dms[0].nact
        framesizex = p_wfs._framesizex
        framesizey = p_wfs._framesizey
        nvalid = p_wfs._nvalid
        if p_wfs.type == WFSType.SH:
            frame = self.wfs.get_binimg(0)
        elif p_wfs.type == WFSType.PYRHR:
            frame = self.wfs.get_pyrimg(0)
        else:
            raise RuntimeError("WFS Type not usable")

        if frame.shape != (framesizex, framesizey):
            raise RuntimeError("framesize not match with the simulation")

        if self.rtc.get_voltage(0).size != nact:
            raise RuntimeError("nact not match with the simulation")

        if self.rtc.get_cmat(0).shape != (nact, nvalid * 2):
            raise RuntimeError("cmat not match with the simulation")

        GFInterfaceWfs = Octopus.getInterfacceClass(p_wfs._frameInterface[0])
        self.fakewfs = GFInterfaceWfs(p_wfs._frameInterface[1])

        self.comp = np.zeros(nact, dtype=np.float32)
        GFInterfaceDms = Octopus.getInterfacceClass(
                self.rtcconf.config.p_dms[0]._actuInterface[0])
        self.fakedms = GFInterfaceDms(self.rtcconf.config.p_dms[0]._actuInterface[1])

        tmp_cmat = self.rtc.get_cmat(0)
        GFInterfaceCmat = Octopus.getInterfacceClass(
                self.rtcconf.config.p_controllers[0]._cmatInterface[0])
        self.cmat = GFInterfaceCmat(
                self.rtcconf.config.p_controllers[0]._cmatInterface[1])
        self.cmat.send(tmp_cmat)

        tmp_valid = self.config.p_wfss[0].get_validsub()
        GFInterfaceVsubs = Octopus.getInterfacceClass(
                self.rtcconf.config.p_wfss[0]._validsubsInterface[0])
        self.valid = GFInterfaceVsubs(
                self.rtcconf.config.p_wfss[0]._validsubsInterface[1])
        if self.rtcconf.config.p_wfss[0].type == WFSType.SH:
            self.valid.send(tmp_valid * self.rtcconf.config.p_wfss[0].npix)
        elif self.rtcconf.config.p_wfss[0].type == WFSType.PYRHR:
            self.valid.send(tmp_valid)
        else:
            raise RuntimeError("WFS Type not usable")

    def next(self, *, move_atmos: bool=True, see_atmos: bool=True, nControl: int=0,
             tar_trace: Iterable[int]=None, wfs_trace: Iterable[int]=None,
             do_control: bool=True, apply_control: bool=True) -> None:
        """
        Overload of the Simulator.next() function
        """
        Simulator.next(self, move_atmos=move_atmos, see_atmos=see_atmos,
                       nControl=nControl, tar_trace=[0], wfs_trace=[0], do_control=False)

        # print("Send a frame")
        p_wfs = self.rtcconf.config.p_wfss[0]
        if p_wfs.type == WFSType.SH:
            frame = self.wfs.get_binimg(0)
        elif p_wfs.type == WFSType.PYRHR:
            frame = self.wfs.get_pyrimg(0)
        else:
            raise RuntimeError("WFS Type not usable")
        self.fakewfs.send(frame)
        if apply_control:
            # print("Wait a command...")
            self.fakedms.recv(self.comp, 0)
            self.dms.set_full_comm(self.comp)
