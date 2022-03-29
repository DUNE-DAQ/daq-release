# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Networkmanager(CMakePackage):
    """Provides an interface for performing network sends and receives directly from DAQ modules"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/networkmanager/"
    url      = "https://github.com/DUNE-DAQ/networkmanager"

    maintainers = ['jcfreeman2']

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/networkmanager")
    version("1.0.2", sha256="dc47f5d45b37026f09c50c21514fc087270c55d35fb717bf83ea599f7878371c", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/networkmanager/legacy.tar.gz/dunedaq-v2.9.0")

    depends_on("ipm")
    depends_on("logging")
    depends_on("utilities")
    depends_on("opmonlib")
    depends_on("ers")
    depends_on("nlohmann-json")
    depends_on("boost")

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
