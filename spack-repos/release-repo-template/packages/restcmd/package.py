# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Restcmd(CMakePackage):
    """HTTP REST backend based CommandFacility"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/restcmd/"
    git =      "https://github.com/DUNE-DAQ/restcmd.git"

    maintainers = ["jcfreeman2"]

    version("VERSION", commit="HASH")

    depends_on("daq-cmake")
    depends_on("cetlib")
    depends_on("logging")
    depends_on("cmdlib")
    depends_on("ers")

    depends_on("nlohmann-json")
    depends_on("pistache@dunedaq-v2.8.0")

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
