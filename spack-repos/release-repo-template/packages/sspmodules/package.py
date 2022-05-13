# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Sspmodules(CMakePackage):
    """SiPM Signal Processor (SSP) readout related modules and utilities"""

    homepage = "XHOMEPAGEX"
    git =      "https://github.com/DUNE-DAQ/sspmodules.git"

    maintainers = ["jcfreeman2"]

    version("XVERSIONX", commit="XHASHX")

    depends_on("appfwk")
    depends_on("iomanager")
    depends_on("logging")
    depends_on("ers")
    depends_on("detdataformats")
    depends_on("readoutlibs")
    depends_on("fdreadoutlibs")
    depends_on("opmonlib")
    depends_on("boost")
    depends_on("daq-cmake", type="build")

    def setup_run_environment(self, env):
        env.set(self.__module__.split(".")[-1].upper().replace("-", "_") + "_SHARE", self.prefix + "/share" )
        env.prepend_path('DUNEDAQ_SHARE_PATH', self.prefix + "/share")
        env.prepend_path('CET_PLUGIN_PATH', self.prefix.lib + "64")
        env.prepend_path("PYTHONPATH", self.prefix.lib + "64/python")


