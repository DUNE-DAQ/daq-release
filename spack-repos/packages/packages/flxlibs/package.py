# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Flxlibs(CMakePackage):
    """DAQ modules, utilities, and scripts for Upstream FELIX Readout Software"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/flxlibs/"
    url =      "https://github.com/DUNE-DAQ/flxlibs"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/flxlibs")
    version("1.3.0", sha256="8f774889551c3784fb5b6fe87baffd48c088e4f919f140c7d6d2a9dcaf59aaea", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/flxlibs/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.2.3", sha256='c36b770072db26dad98f6262bcc15deed7e4af26b0d3aed18dc2b3e2b4fb1c97', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/flxlibs/legacy.tar.gz/dunedaq-v2.8.2")
    version("1.2.1", sha256='e506aa2b7e10aaaadb3fbac951eb28375ada2c6c65e99ccba5842303c3f59d2d', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/flxlibs/legacy.tar.gz/dunedaq-v2.8.0")


    depends_on("daq-cmake")
    depends_on("appfwk")
    depends_on("logging")
    depends_on("ers", when="@1.2.3:")
    depends_on("dataformats", when="@1.2.1")
    depends_on("readout", when="@:1.2.3")
    depends_on("readoutlibs", when="@develop")
    depends_on("readoutlibs", when="@1.3.0:")
    depends_on("fdreadoutlibs", when="@develop")
    depends_on("fdreadoutlibs", when="@1.3.0:")
    depends_on("felix-software")
    depends_on("opmonlib", when="@develop")
    depends_on("opmonlib", when="@1.3.0:")
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

