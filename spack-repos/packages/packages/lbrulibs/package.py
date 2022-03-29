# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Lbrulibs(CMakePackage):
    """DAQ modules, utilities, and scripts for DUNE-ND Upstream DAQ Low Bandwidth Readout Unit"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/lbrulibs/"
    url =      "https://github.com/DUNE-DAQ/lbrulibs"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/lbrulibs")
    version("1.0.6", sha256="46ba037c32b4e29ec0e674b7f4feded7a09d0a4467a637094eb4b172879a5d75", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/lbrulibs/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.0.5", sha256='70171acdacd6e4fedde5b61e0464433d02948b552445205ebe3f60fd22d66eec', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/lbrulibs/legacy.tar.gz/dunedaq-v2.8.2")
    version("1.0.3", sha256='365e361c8736aa365d31e036f6cf7b8b0fc0c0dc04abf3e62bb83838a7afa65b', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/lbrulibs/legacy.tar.gz/dunedaq-v2.8.0")


    depends_on("daq-cmake")
    depends_on("readout", when="@:1.0.5")
    depends_on("readoutlibs", when="@develop")
    depends_on("readoutlibs", when="@1.0.6:")
    depends_on("ndreadoutlibs", when="@develop")
    depends_on("ndreadoutlibs", when="@1.0.6:")
    depends_on("ipm")
    depends_on("appfwk")
    depends_on("logging")
    depends_on("boost")
    depends_on("ers", when="@develop")
    depends_on("ers", when="@1.0.5:")
    depends_on("dataformats", when="@1.0.3")
    depends_on("detdataformats", when="@develop")
    depends_on("detdataformats", when="@1.0.5:")
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

