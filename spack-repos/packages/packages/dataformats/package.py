# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Dataformats(CMakePackage):
    """Raw data reinterpretation utilities"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/dunedaq-v2.8.0/packages/dataformats/"
    url =      "https://github.com/DUNE-DAQ/dataformats"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git=url)

    version("3.0.0", sha256='6da3f65d645792bc022468e27bf16d3f9c4f844fa08a5634e5f6cb1f850cf4ef', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/dataformats/legacy.tar.gz/dunedaq-v2.8.0")

    depends_on("daq-cmake")
    depends_on("ers")
    depends_on('boost' )
    depends_on("logging")

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
