# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class DUNEDAQ(BundlePackage):
    """A dummy package meant to pull in all the packages in the DUNE DAQ suite"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("RELEASE")

    variant('build_type', default='RelWithDebInfo',
            description='The build type to build',
            values=('Debug', 'Release', 'RelWithDebInfo'),
            multi=True)

    depends_on("externals@RELEASE", when="@RELEASE")

    for build_type in ["Debug", "RelWithDebInfo", "Release"]:
        # Folloing lines to be generated from release YAML file
        # depends_on(f"daq-cmake@2.1.0 build_type={build_type}", when=f"build_type={build_type}")