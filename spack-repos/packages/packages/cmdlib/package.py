# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Cmdlib(CMakePackage):
    """Interfaces for commanded objects"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/cmdlib/"
    url =      "https://github.com/DUNE-DAQ/cmdlib"
    
    maintainers = ['jcfreeman2']

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/cmdlib")
    version("1.1.5", sha256="ce8d43e402b9d2abc0a166e11521612f80989c803502868ab6cb2784ab59ba5b", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/cmdlib/legacy.tar.gz/dunedaq-v2.9.0")
    version('1.1.4', sha256='049a6de55b4a53a9c101a268a521cd0c5086acadab8e490d29c8918de3d17723', extension='tar.gz', url="https://codeload.github.com/DUNE-DAQ/cmdlib/legacy.tar.gz/dunedaq-v2.8.0")

    depends_on('daq-cmake')
    depends_on('nlohmann-json')
    depends_on('cetlib')
    depends_on('logging')
    depends_on('boost' )
    depends_on('ers', when="@develop")
    depends_on('ers', when="@1.1.5:")

    depends_on('py-moo', type='build')

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
