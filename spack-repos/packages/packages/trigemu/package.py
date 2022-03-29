# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Trigemu(CMakePackage):
    """Trigger decision emulator for readout application tests"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/trigemu/"
    url =      "https://github.com/DUNE-DAQ/trigemu"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/trigemu")
    version("2.3.2", sha256="fb289c2bae40e0c858ca5c8391168e2f591a64a8513849251c9dca5f18493fea", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/trigemu/legacy.tar.gz/dunedaq-v2.9.0")
    version("2.3.1", sha256='7d3de1e978d9011755905c214174fc69ca55ccc595a016b72aecbd7140871c97', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/trigemu/legacy.tar.gz/dunedaq-v2.8.2")
    version("2.3.0", sha256='3caed5c248c919ccd958b5b910dcdfc25da9e909af4a01e77cd78bc1144dabe2', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/trigemu/legacy.tar.gz/dunedaq-v2.8.0")


    depends_on("daq-cmake")
    depends_on("appfwk")
    depends_on("logging")
    depends_on("dfmessages")
    depends_on("opmonlib", when="@develop")
    depends_on("opmonlib", when="@2.3.2:")

    depends_on('py-moo', type='build')

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


