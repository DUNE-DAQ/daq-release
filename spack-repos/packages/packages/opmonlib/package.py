# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Opmonlib(CMakePackage):
    """Operational monitoring library"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/opmonlib/"
    url =      "https://github.com/DUNE-DAQ/opmonlib"

    maintainers = ['jcfreeman2']

    version("develop", branch="develop", git=url)
    version('1.3.4', sha256='1d867ca1d1b39d2aa651860dd3a7dbb26958ba6b426c4c2a0a60970e32c7052e', extension='tar.gz', url="https://codeload.github.com/DUNE-DAQ/opmonlib/legacy.tar.gz/dunedaq-v2.8.2")
    version('1.3.2', sha256='5f6e170ee6713b94c1a571233e801ea636f6c5baa61a68ab79ee4eedca0a6934', extension='tar.gz', url="https://codeload.github.com/DUNE-DAQ/opmonlib/legacy.tar.gz/dunedaq-v2.8.0")


    depends_on('daq-cmake')
    depends_on('cetlib')
    depends_on('ers')
    depends_on('logging')
    depends_on('nlohmann-json')

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
