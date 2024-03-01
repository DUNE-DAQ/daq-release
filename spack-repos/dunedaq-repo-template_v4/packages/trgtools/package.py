# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Trgtools(CMakePackage):
    """Trigger emulation and analysis tools"""

    homepage = "https://github.com/DUNE-DAQ/trgtools"
    git      = "https://github.com/DUNE-DAQ/trgtools.git"

    maintainers = ['jcfreeman2']

    version("XVERSIONX", commit="XHASHX")

    depends_on("cli11")
    depends_on("fmt")
    depends_on("nlohmann-json")

    depends_on("hdf5libs")
    depends_on("trgdataformats")
    depends_on("triggeralgs")
    depends_on("detchannelmaps")

    depends_on("daq-cmake")

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
