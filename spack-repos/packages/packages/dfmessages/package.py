# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Dfmessages(CMakePackage):
    """Dataflow messages"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/dfmessages/"
    url =      "https://github.com/DUNE-DAQ/dfmessages"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/dfmessages")
    version("2.2.2", sha256="358d2e1ed266a790db34455091fc847d607a9a67e019839f7647062b113f2219", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/dfmessages/legacy.tar.gz/dunedaq-v2.9.0")
    version("2.2.1", sha256='c442229fb445711edc5d4a7a6884ab35e2853a48c890dcdefba81bcb9c159540', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/dfmessages/legacy.tar.gz/dunedaq-v2.8.2")
    version("2.2.0", sha256='80b6b78e1d36c6db19b623152a37138470e64cf750370483e0820bbaaa607603', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/dfmessages/legacy.tar.gz/dunedaq-v2.8.0")

    depends_on("daq-cmake")
    depends_on("nwqueueadapters")
    depends_on("serialization")
    depends_on("dataformats", when="@2.2.0")
    depends_on("daqdataformats", when="@2.2.1:")
    depends_on("daqdataformats", when="@develop")

    depends_on('boost' )

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

