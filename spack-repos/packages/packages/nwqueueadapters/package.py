# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Nwqueueadapters(CMakePackage):
    """DAQ modules that connect appfwk queues to IPM network connections"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/nwqueueadapters/"
    url =      "https://github.com/DUNE-DAQ/nwqueueadapters"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/nwqueueadapters")
    version("1.5.0", sha256="338d1fc42d9fbbdb4e88c117a287ab4cbbd3099086b292c7e1b4264b24551692", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/nwqueueadapters/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.4.0", sha256='76298a304ac50b035bbab4ffc011f91a81037123740a69a0a795e015857298aa', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/nwqueueadapters/legacy.tar.gz/dunedaq-v2.8.0")

    depends_on("daq-cmake")
    depends_on("appfwk")
    depends_on("utilities", when="@develop")
    depends_on("utilities", when="@1.5.0:")
    depends_on("networkmanager", when="@develop")
    depends_on("networkmanager", when="@1.5.0:")
    depends_on("logging")
    depends_on("ipm")
    depends_on("serialization")
    depends_on("opmonlib")
    depends_on("ers", when="@develop")
    depends_on("ers", when="@1.5.0:")
    depends_on("boost", when="@develop")
    depends_on("boost", when="@1.5.0:")
    depends_on("py-moo", type='build')

    depends_on("nlohmann-json")

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
