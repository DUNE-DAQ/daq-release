# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Trigger(CMakePackage):
    """Modules that make up the DUNE FD DAQ trigger system"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/trigger/"
    url =      "https://github.com/DUNE-DAQ/trigger"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/trigger")
    version("1.2.0", sha256="37a91307ee6900528173f0ecfd8a5cb3f10c2a8d7ba8ebc83993f2ef9eabd418", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/trigger/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.1.3", sha256='cdbf0758aea8c0c46f3ac34b41e3ba84adb23a8cbdf396c1889cc433bac1dcfb', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/trigger/legacy.tar.gz/dunedaq-v2.8.2")
    version("1.1.2", sha256='1d0e03e377ca975f5f9889109abe3d32acd83c980e80227f3d687f3730387995', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/trigger/legacy.tar.gz/dunedaq-v2.8.0")


    depends_on("daq-cmake")
    depends_on("ers")
    depends_on("serialization")
    depends_on("logging")
    depends_on("appfwk")
    depends_on("triggeralgs")
    depends_on("dfmessages")
    depends_on("timinglibs")
    depends_on("nwqueueadapters")
    depends_on("utilities", when="@develop")
    depends_on("utilities", when="@1.2.0:")
    depends_on("networkmanager", when="@develop")
    depends_on("networkmanager", when="@1.2.0:")
    depends_on("dataformats", when="@1.1.2")
    depends_on("daqdataformats", when="@1.1.3:")
    depends_on("daqdataformats", when="@develop")
    depends_on("detdataformats", when="@develop")
    depends_on("hdf5libs", when="@develop")
    depends_on("cli11", when="@develop")

    
    depends_on('boost' )
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

