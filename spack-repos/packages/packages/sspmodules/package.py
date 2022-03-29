# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Sspmodules(CMakePackage):
    """SiPM Signal Processor (SSP) readout related modules and utilities"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/sspmodules/"
    url =      "https://github.com/DUNE-DAQ/sspmodules"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/sspmodules")
    version("1.1.0", sha256="e5b102111f78975c2da3c3ec7336cbfe50e3cf7438d28939bfa29beedb211d07", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/sspmodules/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.0.2", sha256='e92a55c0367489c1d1390736722009c07df296b6dbe18d1a2cb984d877eea1a9', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/sspmodules/legacy.tar.gz/dunedaq-v2.8.2")

    depends_on("appfwk")
    depends_on("logging")
    depends_on("ers", when="@develop")
    depends_on("ers", when="@1.1.0:")
    depends_on("detdataformats")
    depends_on("readout", when="@1.0.2")
    depends_on("readoutlibs", when="@develop")
    depends_on("readoutlibs", when="@1.1.0:")
    depends_on("fdreadoutlibs", when="@develop")
    depends_on("fdreadoutlibs", when="@1.1.0:")
    depends_on("opmonlib", when="@develop")
    depends_on("opmonlib", when="@1.1.0:")
    depends_on("daq-cmake")
    depends_on('py-moo', type='build')

    def setup_run_environment(self, env):
        env.set(self.__module__.split(".")[-1].upper().replace("-", "_") + "_SHARE", self.prefix + "/share" )
        env.prepend_path('DUNEDAQ_SHARE_PATH', self.prefix + "/share")
        env.prepend_path('CET_PLUGIN_PATH', self.prefix.lib + "64")
        env.prepend_path("PYTHONPATH", self.prefix.lib + "64/python")


