## @package   shesha.supervisor.components.coronograph
## @brief     User layer for initialization and execution of a COMPASS simulation
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


from shesha.supervisor.components.coronagraph.coronagraphCompass import CoronagraphCompass
from shesha.supervisor.components.coronagraph.genericCoronagraph import GenericCoronagraph
from shesha.supervisor.components.coronagraph.perfectCoronagraph import PerfectCoronagraphCompass
from shesha.supervisor.components.coronagraph.stellarCoronagraph import StellarCoronagraphCompass

__all__ = ['CoronagraphCompass', 'GenericCoronagraph', 'PerfectCoronagraphCompass', 'StellarCoronagraphCompass']