## @package   shesha.util
## @brief     Shesha utilities
## @author    COSMIC Team <https://github.com/COSMIC-RTC/compass>
## @date      2022/01/24
## @copyright 2011-2024 COSMIC Team <https://github.com/COSMIC-RTC/compass>
#
# This file is part of COMPASS <https://github.com/COSMIC-RTC/compass>

# COMPASS is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser
# General Public License as published by the Free Software Foundation, either version 3 of the 
# License, or any later version.

# COMPASS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
# See the GNU Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License along with COMPASS. 
# If not, see <https://www.gnu.org/licenses/>

# Copyright (C) 2011-2024 COSMIC Team <https//://github.com/COSMIC-RTC/compass>
import numpy as np

def write_general(file_name, geom, controllers, tel, simul_name):
    """Write (append) general simulation parameter to file for YAO use

    Args:
        file_name : (str) : name of the file to append the parameter to

        geom : (ParamGeom) : compass AO geometry parameters

        controllers : ([ParamController]) : list of compass controller parameters

        tel : (ParamTel) : compass telescope parameters

        simul_name : (str) : simulation name
    """
    f = open(file_name,"w")
    f.write("\n\n//------------------------------")
    f.write("\n//general parameters")
    f.write("\n//------------------------------")
    f.write("\nsim.name        = \"" + simul_name + "\";")
    f.write("\nsim.pupildiam   = " + str(geom.pupdiam) + ";")
    f.write("\nsim.debug       = 0;")
    f.write("\nsim.verbose     = 1;")

    f.write("\nmat.file            = \"\";")
    f.write("\nmat.condition = &(" + np.array2string( \
            np.array([np.sqrt(c.maxcond) for c in controllers]), \
            separator=',',max_line_width=300) + ");")

    f.write("\nmat.method = \"none\";")
    #f.write("\nhfield = 15")
    f.write("\nYAO_SAVEPATH = \"\"; // where to save the output to the simulations")

    f.write("\ntel.diam = " + str(tel.diam) + ";")
    f.write("\ntel.cobs = " + str(tel.cobs) + ";")
    f.write("\ndm       = [];")
    f.write("\nwfs      = [];")
