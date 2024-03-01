# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Hermesmodules(CMakePackage):
    """The HERMES core modules and control libraries."""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/hermesmodules/"
    git =      "https://github.com/DUNE-DAQ/hermesmodules.git"

    maintainers = ["jcfreeman2"]

    version("XVERSIONX", commit="XHASHX")


    depends_on('daq-cmake')

    depends_on('fmt')
    depends_on('uhal')
    depends_on('ers')
    depends_on('appdal')
    depends_on('appfwk')
    depends_on('coredal')
    depends_on('opmonlib')


    depends_on('py-moo', type='build')
    depends_on('py-pybind11')
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
