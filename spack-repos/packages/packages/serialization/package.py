# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Serialization(CMakePackage):
    """Utilities for C++ object serialization/deserialization"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/serialization/"
    url =      "https://github.com/DUNE-DAQ/serialization"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/serialization")
    version("1.2.3", sha256="2ac9a5277225fa8a2c8f4616f6d2a2d273ca30bbf713f34b976d4ea8eb265c51", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/serialization/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.2.2", sha256='008cf64f9ede712cca240908dd181e28008b510f2042a343f0079ae2c7640afe', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/serialization/legacy.tar.gz/dunedaq-v2.8.0")

    depends_on("daq-cmake")
    depends_on("appfwk")
    depends_on("logging")
    depends_on("ipm", when="@1.2.2")
    depends_on("ers", when="@develop")
    depends_on("ers", when="@1.2.3:")
    depends_on("boost")

    depends_on("py-moo", type='build')

    depends_on("msgpack-c")
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
