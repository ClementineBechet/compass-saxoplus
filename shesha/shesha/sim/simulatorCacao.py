## @package   shesha.sim.simulatorCacao
## @brief     Class SimulatorCacao: Cacao overloaded simulator
## @author    COMPASS Team <https://github.com/ANR-COMPASS>
## @version   4.3.2
## @date      2011/01/28
## @copyright GNU Lesser General Public License
#
#  This file is part of COMPASS <https://anr-compass.github.io/compass/>
#
#  Copyright (C) 2011-2019 COMPASS Team <https://github.com/ANR-COMPASS>
#  All rights reserved.
#  Distributed under GNU - LGPL
#
#  COMPASS is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser 
#  General Public License as published by the Free Software Foundation, either version 3 of the License, 
#  or any later version.
#
#  COMPASS: End-to-end AO simulation tool using GPU acceleration 
#  The COMPASS platform was designed to meet the need of high-performance for the simulation of AO systems. 
#  
#  The final product includes a software package for simulating all the critical subcomponents of AO, 
#  particularly in the context of the ELT and a real-time core based on several control approaches, 
#  with performances consistent with its integration into an instrument. Taking advantage of the specific 
#  hardware architecture of the GPU, the COMPASS tool allows to achieve adequate execution speeds to
#  conduct large simulation campaigns called to the ELT. 
#  
#  The COMPASS platform can be used to carry a wide variety of simulations to both testspecific components 
#  of AO of the E-ELT (such as wavefront analysis device with a pyramid or elongated Laser star), and 
#  various systems configurations such as multi-conjugate AO.
#
#  COMPASS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the 
#  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
#  See the GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License along with COMPASS. 
#  If not, see <https://www.gnu.org/licenses/lgpl-3.0.txt>.


import sys
import os

from .simulator import Simulator
from shesha.init.rtc_init import rtc_init


class SimulatorCacao(Simulator):
    """
        Class SimulatorCacao: Cacao overloaded simulator
        _tar_init and _rtc_init to instantiate Cacao classes instead of regular classes
        next() to call rtc/tar.publish()
    """

    def _rtc_init(self, ittime) -> None:
        '''
        Initializes a Rtc_Cacao object
        '''
        if self.config.p_controllers is not None or self.config.p_centroiders is not None:
            print("->rtc")
            #   rtc
            self.rtc = rtc_init(self.c, self.tel, self.wfs, self.dms, self.atm,
                                self.config.p_wfss, self.config.p_tel,
                                self.config.p_geom, self.config.p_atmos, ittime,
                                self.config.p_centroiders, self.config.p_controllers,
                                self.config.p_dms, cacao=True, tar=None)
        else:
            self.rtc = None

    def next(self, **kwargs) -> None:
        """
        Overload of the Simulator.next() function with cacao publications
        """
        self.rtc.d_centro[0].load_img(self.wfs.d_wfs[0].d_binimg)
        Simulator.next(self, **kwargs)
        if self.rtc is not None:
            self.rtc.publish()
