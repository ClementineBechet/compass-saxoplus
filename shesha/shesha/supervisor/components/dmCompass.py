## @package   shesha.supervisor
## @brief     User layer for initialization and execution of a COMPASS simulation
## @author    COMPASS Team <https://github.com/ANR-COMPASS>
## @version   5.0.0
## @date      2020/05/18
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
from shesha.init.dm_init import dm_init
import numpy as np

class DmCompass(object):
    """ DM handler for compass simulation

    Attributes:
        dms : (sutraWrap.Dms) : Sutra dms instance

        context : (carmaContext) : CarmaContext instance

        config : (config module) : Parameters configuration structure module    
    """
    def __init__(self, context, config):
        """ Initialize a DmCompass component for DM related supervision

        Parameters:
            context : (carmaContext) : CarmaContext instance

            config : (config module) : Parameters configuration structure module
        """
        self.context = context
        self.config = config # Parameters configuration coming from supervisor init
        print("->dms init")
        self.dms = dm_init(self.context, self.config.p_dms, self.config.p_tel,
                               self.config.p_geom, self.config.p_wfss)

    def set_command(self, commands: np.ndarray) -> None:
        """ Immediately sets provided command to DMs - does not affect integrator

        Parameters:
            commands : (np.ndarray) : commands vector to apply
        """
        self.dms.set_full_com(commands)

    def set_one_actu(self, dm_index: int, nactu: int, *, ampli: float = 1) -> None:
        """ Push the selected actuator

        Parameters:
            dm_index : (int) : DM index

            nactu : (int) : actuator index to push

            ampli : (float, optional) : amplitude to apply. Default is 1 volt
        """
        self.dms.d_dms[dm_index].comp_oneactu(nactu, ampli)

    def get_influ_function(self, dm_index : int) -> np.ndarray:
        """ Returns the influence function cube for the given dm

        Parameters:
            dm_index : (int) : index of the DM

        Return:
            influ : (np.ndarray) : Influence functions of the DM dm_index
        """
        return self.config.p_dms[dm_index]._influ

    def get_influ_function_ipupil_coords(self, dm_index : int) -> np.ndarray:
        """ Returns the lower left coordinates of the influ function support in the ipupil coord system

        Parameters:
            dm_index : (int) : index of the DM

        Return:
            coords : (tuple) : (i, j)
        """
        i1 = self.config.p_dm0._i1  # i1 is in the dmshape support coords
        j1 = self.config.p_dm0._j1  # j1 is in the dmshape support coords
        ii1 = i1 + self.config.p_dm0._n1  # in  ipupil coords
        jj1 = j1 + self.config.p_dm0._n1  # in  ipupil coords
        return ii1, jj1

    def reset_dm(self, dm_index: int = -1) -> None:
        """ Reset the specified DM or all DMs if dm_index is -1

        Parameters:
            dm_index : (int, optional) : Index of the DM to reset
                                         Default is -1, i.e. all DMs are reset
        """
        if (dm_index == -1):  # All Dms reset
            for dm in self.dms.d_dms:
                dm.reset_shape()
        else:
            self.dms.d_dms[dm_index].reset_shape()

    def get_dm_shape(self, indx : int) -> np.ndarray:
        """ Return the current phase shape of the selected DM

        Parameters:
            indx : (int) : Index of the DM

        Return:
            dm_shape : (np.ndarray) : DM phase screen

        """
        return np.array(self.dms.d_dms[indx].d_shape)

    def set_dm_registration(self, dm_index : int, *, dx : float=None, dy : float=None, 
                            theta : float=None, G : float=None) -> None:
        """Set the registration parameters for DM #dm_index

        Parameters:
            dm_index : (int) : DM index

            dx : (float, optionnal) : X axis registration parameter [meters]. If None, re-use the last one

            dy : (float, optionnal) : Y axis registration parameter [meters]. If None, re-use the last one

            theta : (float, optionnal) : Rotation angle parameter [rad]. If None, re-use the last one

            G : (float, optionnal) : Magnification factor. If None, re-use the last one
        """
        if dx is not None:
            self.config.p_dms[dm_index].set_dx(dx)
        if dy is not None:
            self.config.p_dms[dm_index].set_dy(dy)
        if theta is not None:
            self.config.p_dms[dm_index].set_theta(theta)
        if G is not None:
            self.config.p_dms[dm_index].set_G(G)

        self.dms.d_dms[dm_index].set_registration(
                self.config.p_dms[dm_index].dx / self.config.p_geom._pixsize,
                self.config.p_dms[dm_index].dy / self.config.p_geom._pixsize,
                self.config.p_dms[dm_index].theta, self.config.p_dms[dm_index].G)
