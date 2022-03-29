# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Triggeralgs(CMakePackage):
    """Trigger algorithms"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/triggeralgs/"
    url =      "https://github.com/DUNE-DAQ/triggeralgs"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/triggeralgs")
    version("0.3.3", sha256="6da270a7a770d48f317b0b5917cc34628cb32eb31b81253e17ec95b32314e4ad", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/triggeralgs/legacy.tar.gz/dunedaq-v2.9.0")
    version("0.3.1", sha256='379dd49081e9443abd856a0bb6fddee1c5f06769be7a29371e48ddc843f50f66', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/triggeralgs/legacy.tar.gz/dunedaq-v2.8.2")
    version("0.3.0", sha256='51fbdbc71b77f1bfc0c80d7eb766b9b58bbf55186997cdd5d34cf859e02c31da', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/triggeralgs/legacy.tar.gz/dunedaq-v2.8.0")


    depends_on("nlohmann-json")
    depends_on("trace")
    depends_on("detdataformats")

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

