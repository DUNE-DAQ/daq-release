# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Timing(CMakePackage):
    """C++ interface to the timing firmware"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/timing/"
    url =      "https://github.com/DUNE-DAQ/timing"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/timing")
    version("6.0.0", sha256="c77b9b5d3759aed717ec4fc611a816a774559149612284c1d46de48aa2084185", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/timing/legacy.tar.gz/dunedaq-v2.9.0")
    version("5.7.0", sha256='5c06342746aac7e6ffb11df8dc2357b748ec88995ae97ecd1593e0f628f02abf', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/timing/legacy.tar.gz/dunedaq-v2.8.2")
    version("5.5.1", sha256='17ef87db84ff85fecae32b01ceb755a5e43af46a883ed6fab3653e1cfaa6bc09', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/timing/legacy.tar.gz/dunedaq-v2.8.0")


    depends_on('daq-cmake')
    depends_on('logging')
    depends_on('ers')
    depends_on('opmonlib')

    depends_on('nlohmann-json')
    depends_on('uhal')
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

