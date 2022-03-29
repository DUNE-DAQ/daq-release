# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Influxopmon(CMakePackage):
    """Influx database based plugin for operational monitoring"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/influxopmon/"
    url =      "https://github.com/DUNE-DAQ/influxopmon"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/influxopmon")
    version("1.5.3", sha256="b21c4a6959c451f4c91367a70057a9c1797bd92fd31680e302d39b3edd1d94e9", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/influxopmon/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.5.2", sha256='17b4f7a9159af34373572e47a8d980d7eaa14611a77bb737351b93a6a4681b40', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/influxopmon/legacy.tar.gz/dunedaq-v2.8.2")
    version("1.4.0", sha256='3ed4818efe6af45b8d4fe6bdb552ceeeed8b83c58e03f49d1abfa5bce5816c8a', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/influxopmon/legacy.tar.gz/dunedaq-v2.8.0")



    depends_on("daq-cmake")
    depends_on("opmonlib")
    depends_on("cpr")
    depends_on("curl")
    depends_on("ers", when="@develop")
    depends_on("ers", when="@1.5.3:")
    depends_on("nlohmann-json")
    depends_on("logging")

    # DBT_DEBUG is used by daq-cmake to set compiler options 
    def cmake_args(self): 
        if str(self.spec.variants['build_type']) == "build_type=Debug": 
            return ["-DDBT_DEBUG=true"] 
        else: 
            return ["-DDBT_DEBUG=false"] 

    def setup_run_environment(self, env):
        env.set(self.__module__.split(".")[-1].upper().replace("-", "_") + "_SHARE", self.prefix + "/share" )
        env.prepend_path('DUNEDAQ_SHARE_PATH', self.prefix + "/share")
        env.prepend_path('CET_PLUGIN_PATH', self.prefix.lib + "64")
        env.prepend_path("PYTHONPATH", self.prefix.lib + "64/python")


