# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Hdf5libs(CMakePackage):
    """FIXME: Put a proper description of your package here."""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/hdf5libs/"
    url =      "https://github.com/DUNE-DAQ/hdf5libs"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/hdf5libs")
    version("1.0.3", sha256="e84d203ff7a255d0620e3a1b1897e0acfe1791a2206c9b60b762239ed68b9b48", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/hdf5libs/legacy.tar.gz/dunedaq-v2.9.0")
    version("1.0.1", sha256='619d3f65089f1c6bddc78073e75ab61489a4fd0fb843ff4f7fabda30f96876d6', extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/hdf5libs/legacy.tar.gz/dunedaq-v2.8.2")

    depends_on("logging")
    depends_on("highfive ~mpi")
    depends_on("daqdataformats")
    depends_on("detdataformats")
    depends_on("detchannelmaps")
    depends_on("cetlib")
    depends_on("ers", when="@develop")
    depends_on("ers", when="@1.0.3:")
    depends_on("daq-cmake")
    depends_on('py-moo', type='build')
    depends_on("boost")
    depends_on("nlohmann-json")

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


