# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Erskafka(CMakePackage):
    """The erskafka plugin"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/erskafka/"
    url =      "https://github.com/DUNE-DAQ/erskafka"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/erskafka")
    version("1.3.3", sha256="83920a48200e20f420be1b8225c4f6bc80efeb35c7f168caf72fe884be8223aa", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/erskafka/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.3.0", sha256='166dbdc9cf1b37d9b1c0ae89fac977d113ebb90dd6b72499a85ab90b814b3100', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/erskafka/legacy.tar.gz/dunedaq-v2.8.0")

    depends_on("daq-cmake")
    depends_on("ers")
    depends_on("librdkafka")
    depends_on('boost' )
    
    depends_on("openssl")
    depends_on("cyrus-sasl")
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

