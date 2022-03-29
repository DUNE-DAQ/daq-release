# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class DaqCmake(CMakePackage):
    """CMake support for DUNE-DAQ packages"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/"
    url      = "https://github.com/DUNE-DAQ/daq-cmake"

    maintainers = ["jcfreeman2"]

    version("develop", branch="develop", git="https://github.com/DUNE-DAQ/daq-cmake")
    version('2.1.0', sha256='5fa145804846d582e95d4ec4eb1157ff1d30b1352811c75ad0bd17bf5be9bf19', extension='tar.gz', url="https://codeload.github.com/DUNE-DAQ/daq-cmake/legacy.tar.gz/dunedaq-v2.9.0")

    depends_on("py-pybind11")

    # DBT_DEBUG is used by daq-cmake to set compiler options 
    def cmake_args(self): 
        if str(self.spec.variants['build_type']) == "build_type=Debug": 
            return ["-DDBT_DEBUG=true"] 
        else: 
            return ["-DDBT_DEBUG=false"] 

    def setup_run_environment(self, env):
        env.prepend_path('PYTHONPATH', self.prefix.lib + "64/python")
