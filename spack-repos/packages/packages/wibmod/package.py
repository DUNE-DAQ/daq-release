# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Wibmod(CMakePackage):
    """WIB configuration and monitoring interface for DUNE's appfwk"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/wibmod/"
    url =      "https://github.com/DUNE-DAQ/wibmod"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/wibmod")
    version("1.2.5", sha256="e84d203ff7a255d0620e3a1b1897e0acfe1791a2206c9b60b762239ed68b9b48", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/wibmod/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.2.3", sha256='b61aed5c68eb51d8559df3e8bc19e7971bbe16b26d29aa22312f98020f28c266', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/wibmod/legacy.tar.gz/dunedaq-v2.8.2")

    depends_on("ers")
    depends_on("logging")
    depends_on("serialization", when="@1.2.3")
    depends_on("appfwk")
    depends_on("cppzmq")
    depends_on("protobuf")
    depends_on("highfive ~mpi", when="@1.2.5:")
    depends_on("daqdataformats", when="@1.2.5:")
    depends_on("detdataformats", when="@1.2.5:")
    depends_on("detchannelmaps", when="@1.2.5:")

    depends_on("daq-cmake")
    depends_on('py-moo', type='build')

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



