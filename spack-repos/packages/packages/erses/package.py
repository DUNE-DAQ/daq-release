# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Erses(CMakePackage):
    """Insert ERS messages into a searchable database"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/dunedaq-v2.8.0/packages/erses/"
    url =      "https://github.com/DUNE-DAQ/erses"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git=url)

    version("1.0.0", sha256='cc171ad7ad870901b296d4c814f96efe732cb332611fb5d5b5ee7d2e52775168', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/erses/legacy.tar.gz/dunedaq-v2.8.0")

    depends_on("daq-cmake")
    depends_on("ers")
    depends_on("cpr")

    depends_on("nlohmann-json")

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


