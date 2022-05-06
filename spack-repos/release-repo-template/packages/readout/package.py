# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Readout(CMakePackage):
    """Upstream DAQ readout, DAQ modules, CCM interface implementations"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/dunedaq-v2.8.0/packages/readout/"
    git =      "https://github.com/DUNE-DAQ/readout.git"

    maintainers = ["jcfreeman2"]

    version("XVERSIONX", commit="XHASHX")


    depends_on("daq-cmake", type="build")
    depends_on("trigger")
    depends_on("timinglibs")
    depends_on("triggeralgs")
    depends_on("dfmessages")
    depends_on("appfwk")
    depends_on("folly")
    depends_on("dataformats", when="@1.4.2")
    depends_on("daqdataformats", when="@1.4.5:")
    depends_on("detdataformats", when="@1.4.5:")
    depends_on("ers", when="@1.45:")
    depends_on("opmonlib")
    depends_on("logging")

    depends_on('boost')
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

