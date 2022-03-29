# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Utilities(CMakePackage):
    """Self-explanatory"""

    homepage = "https://github.com/DUNE-DAQ/utilities"
    url      = "https://github.com/DUNE-DAQ/utilities"

    maintainers = ['jcfreeman2']

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/utilities")
    version("1.0.0", sha256="b5a564fcc2a3ba5816048f838719b652e259e4ddb255e7c34df89af885c0265f", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/utilities/legacy.tar.gz/dunedaq-v2.9.0")

    depends_on("nlohmann-json")
    depends_on("logging")
    depends_on("boost")
    depends_on("ers")

    depends_on("py-moo", type='build')
    depends_on("daq-cmake")

    # DBT_DEBUG is used by daq-cmake to set compiler options 
    def cmake_args(self): 
        if str(self.spec.variants['build_type']) == "build_type=Debug": 
            return ["-DDBT_DEBUG=true"] 
        else: 
            return ["-DDBT_DEBUG=false"] 

    def setup_run_environment(self, env):
        env.set(self.__module__.split(".")[-1].upper().replace("-", "_") + "_SHARE", self.prefix + "/share" )
        env.prepend_path("DUNEDAQ_SHARE_PATH", self.prefix + "/share")
        env.prepend_path('CET_PLUGIN_PATH', self.prefix.lib + "64")
        env.prepend_path("PYTHONPATH", self.prefix.lib + "64/python")
