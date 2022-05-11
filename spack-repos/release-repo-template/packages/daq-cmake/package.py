# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class DaqCmake(CMakePackage):
    """CMake support for DUNE-DAQ packages"""

    homepage = "XHOMEPAGEX"
    git      = "https://github.com/DUNE-DAQ/daq-cmake.git"

    maintainers = ["jcfreeman2"]

    version("XVERSIONX", commit="XHASHX")

    variant('dev', default=True, description='Load in tools used to develop DUNE DAQ packages')

    depends_on("py-pybind11", type=("link", "run"), when="+dev")
    depends_on("py-moo", type=("link", "run"), when="+dev")
    depends_on("cmake", type=("build", "link", "run"), when="+dev")
    
    # DBT_DEBUG is used by daq-cmake to set compiler options
    def cmake_args(self):
        if str(self.spec.variants['build_type']) == "build_type=Debug":
            return ["-DDBT_DEBUG=true"]
        else:
            return ["-DDBT_DEBUG=false"]

    def setup_run_environment(self, env):
        env.prepend_path('PYTHONPATH', self.prefix.lib + "64/python")
