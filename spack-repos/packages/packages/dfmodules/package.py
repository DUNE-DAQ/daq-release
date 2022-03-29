# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Dfmodules(CMakePackage):
    """Dataflow Applications"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/dfmodules/"
    url =      "https://github.com/DUNE-DAQ/dfmodules"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/dfmodules")
    version("2.4.0", sha256="3cb01817201e93eb3f9d821683d1ea7de8f3c28a115a8e48744d05348dc609ec", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/dfmodules/legacy.tar.gz/dunedaq-v2.9.0")
    version("2.3.2", sha256='675989875d3e4effcc92ea9f9263dc9e456a63f57ea4ba5eb3cce1122141061b', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/dfmodules/legacy.tar.gz/dunedaq-v2.8.2")
    version("2.2.1", sha256='c62c967508b0b24101eb529be47311ea6c0a26efa55f3f5a59a4942acff073a4', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/dfmodules/legacy.tar.gz/dunedaq-v2.8.0")


    depends_on("daq-cmake")
    depends_on("readout", when="@:2.3.2")
    depends_on("trigger")
    depends_on("triggeralgs")
    depends_on("timinglibs", when="@:2.3.2")
    depends_on("dfmessages")
    depends_on("serialization")
    depends_on("appfwk")
    depends_on("opmonlib")
    depends_on("networkmanager", when="@develop")
    depends_on("networkmanager", when="@2.4.0:")
    depends_on("readoutlibs", when="@develop")
    depends_on("readoutlibs", when="@2.4.0:")
    depends_on("ers", when="@develop")
    depends_on("ers", when="@2.4.0:")
    
    depends_on("logging")
    depends_on("py-moo", type='build')

    depends_on("highfive ~mpi")
    depends_on("boost")

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

