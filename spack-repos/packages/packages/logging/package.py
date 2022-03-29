# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Logging(CMakePackage):
    """Contains the functions DUNE DAQ packages use to output text"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/logging/"
    url =      "https://github.com/DUNE-DAQ/logging"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/logging")
    version("1.0.5", sha256="c1eaa50cc0748ea318b69ebd01ff9d73e9e7c233e64143867e10718a6bac8f29", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/logging/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.0.3", sha256='fdf0e8a6ca4f223c584e18af25fad146401d1b6a2545cc62c6b1787e92d271f5', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/logging/legacy.tar.gz/dunedaq-v2.8.0")

    depends_on('daq-cmake')
    depends_on('trace')
    depends_on('ers')

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

