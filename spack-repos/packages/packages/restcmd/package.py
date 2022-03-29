# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Restcmd(CMakePackage):
    """HTTP REST backend based CommandFacility"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/restcmd/"
    url =      "https://github.com/DUNE-DAQ/restcmd"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/restcmd")
    version("1.1.4", sha256="0ebf47608eb38185910f35616a44cd462778e01d0b7628f26cbb05bb473e8855", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/restcmd/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.1.3", sha256='840b5a0e115e9699100706b860c7438213917b401f7881ab3f60fe6179b9cfd2', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/restcmd/legacy.tar.gz/dunedaq-v2.8.0")

    depends_on("daq-cmake")
    depends_on("cetlib")
    depends_on("logging")
    depends_on("cmdlib")
    depends_on("ers", when="@develop")
    depends_on("ers", when="@1.1.4:")

    depends_on("nlohmann-json")
    depends_on("pistache@dunedaq-v2.8.0")

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
