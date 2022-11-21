# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Fdreadoutlibs(CMakePackage):
    """Classes for working with far detector data (WIB, SSP, etc.)"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/fdreadoutlibs/"
    git      = "https://github.com/DUNE-DAQ/fdreadoutlibs.git"

    maintainers = ['jcfreeman2']

    version("XVERSIONX", commit="XHASHX")

    depends_on("ers")
    depends_on("iomanager")
    depends_on("appfwk")
    depends_on("logging")
    depends_on("opmonlib")
    depends_on("readoutlibs")
    depends_on("daqdataformats")
    depends_on("detchannelmaps")
    depends_on("detdataformats")
    depends_on("trigger")
    depends_on("triggeralgs")
    depends_on('folly cxxstd=2a')
    depends_on("boost")

    depends_on("daq-cmake")
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
