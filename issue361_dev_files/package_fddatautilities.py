# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Fddatautilities(BundlePackage):
    """A dummy package meant to pull in packages used for analysis after data is written"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("NFDDU_DEV_240408_A9")

    variant('build_type', default='RelWithDebInfo',
            description='The build type to build',
            values=('Debug', 'Release', 'RelWithDebInfo'),
            multi=True)

    def setup_run_environment(self, env):
        env.set('DUNE_DAQ_BASE_RELEASE', "NFDDU_DEV_240408_A9")


    for build_type in ["Debug", "RelWithDebInfo", "Release"]:
        depends_on(f"coredaq@NB_DEV_240408_A9 subset=datautilities build_type={build_type}", when=f"build_type={build_type}")
        depends_on(f"rawdatautils@NFDDU_DEV_240408_A9 build_type={build_type}", when=f"build_type={build_type}")
        depends_on(f"fddetdataformats@NFDDU_DEV_240408_A9 build_type={build_type}", when=f"build_type={build_type}")
