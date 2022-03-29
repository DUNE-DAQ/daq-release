# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Detchannelmaps(CMakePackage):
    """DUNE detectors channel maps"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/detchannelmaps/"
    url =      "https://github.com/DUNE-DAQ/detchannelmaps"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/detchannelmaps")
    version("1.0.3", sha256="005f3f7f09726bcdd043e9478892a8206d7268e35a59ea91a8589b771d0e00a5", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/detchannelmaps/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.0.2", sha256='e0e673b6772c99354ea4347fbb9f61e2a1b32b072ea5e13d7f66341fa21e3c70', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/detchannelmaps/legacy.tar.gz/dunedaq-v2.8.2")

    depends_on("daq-cmake")
    depends_on("cetlib")
    depends_on("logging")
    depends_on("ers")
    depends_on('py-moo', type='build')

    # DBT_DEBUG is used by daq-cmake to set compiler options 
    def cmake_args(self): 
        if str(self.spec.variants['build_type']) == "build_type=Debug": 
            return ["-DDBT_DEBUG=true"] 
        else: 
            return ["-DDBT_DEBUG=false"] 

    def setup_run_environment(self, env):
        env.set(self.__module__.split(".")[-1].upper().replace("-", "_") + "_SHARE", self.prefix + "/share" )
        env.prepend_path('DUNEDAQ_SHARE_PATH', self.prefix + "/share")
        env.prepend_path('CET_PLUGIN_PATH', self.prefix.lib + "64")
        env.prepend_path("PYTHONPATH", self.prefix.lib + "64/python")


