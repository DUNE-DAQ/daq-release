# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Ndreadoutlibs(CMakePackage):
    """No documentation yet (Dec-11-2021)"""

    homepage = "https://github.com/DUNE-DAQ/ndreadoutlibs"
    url      = "https://github.com/DUNE-DAQ/ndreadoutlibs"

    maintainers = ['jcfreeman2']

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/ndreadoutlibs")
    version("1.0.0", sha256="e905da00add6cdc8bf650f2571c29b612542d12ec688c10bfd95d79bbc5fbe5e", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/ndreadoutlibs/legacy.tar.gz/dunedaq-v2.9.0")

    depends_on("ers")
    depends_on("appfwk")
    depends_on("logging")
    depends_on("opmonlib")
    depends_on("readoutlibs")
    depends_on("daqdataformats")
    depends_on("detdataformats")
    depends_on('folly cxxstd=17')
    depends_on("boost")

    depends_on("daq-cmake")
    depends_on("py-moo", type='build')

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
