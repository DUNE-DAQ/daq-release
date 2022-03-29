# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Detdataformats(CMakePackage):
    """DUNE detectors data formats"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/detdataformats/"
    url =      "https://github.com/DUNE-DAQ/detdataformats"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/detdataformats")
    version("3.2.1", sha256='98076a676276ece40d11d4dd6429e1d0d18509a0ca91bcf61a219003b8f219a0', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/detdataformats/legacy.tar.gz/dunedaq-v2.8.2")

    depends_on("daq-cmake")
    depends_on("boost")
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


