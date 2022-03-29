# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Ipm(CMakePackage):
    """Message passing between processes"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/ipm/"
    url =      "https://github.com/DUNE-DAQ/ipm"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/ipm")
    version("2.3.0", sha256="a4e2e08d0dc3624f6d76e911c211d32f40392bacbe8c34908ea81a69fa889f02", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/ipm/legacy.tar.gz/dunedaq-v2.9.0")
    version("2.2.0", sha256='4c907785d5edfc9108a990653be4b991b25db2e594c6a31439cf5a1631a24c90', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/ipm/legacy.tar.gz/dunedaq-v2.8.0")

    depends_on("daq-cmake")
    depends_on("appfwk", when="@:2.2.0")
    depends_on("logging")
    depends_on("ers")
    depends_on("cetlib", when="@develop")
    depends_on("cetlib", when="@2.3.0:")
    depends_on("boost", when="@develop")
    depends_on("boost", when="@2.3.0:")

    depends_on("cppzmq")
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

