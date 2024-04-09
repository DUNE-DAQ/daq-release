# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Dunedaq(BundlePackage):
    """A dummy package meant to pull in all the packages in the DUNE DAQ suite"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("NB_DEV_240408_A9")

    variant('build_type', default='RelWithDebInfo',
            description='The build type to build',
            values=('Debug', 'Release', 'RelWithDebInfo'),
            multi=True)

    variant('subset', values=('all', 'datautilities'), default='all', description='Select subset of total available dunedaq packages')

    depends_on("externals@NB_DEV_240408_A9", when="@NB_DEV_240408_A9")

    def setup_run_environment(self, env):
        env.set('DUNE_DAQ_BASE_RELEASE', "NFDDU_DEV_240408_A9")


    for build_type in ["Debug", "RelWithDebInfo", "Release"]:
        depends_on(f"daq-cmake@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"ers@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"logging@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"cmdlib@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"restcmd@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"opmonlib@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"rcif@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"appfwk@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"listrev@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"utilities@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"erskafka@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"kafkaopmon@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"ipm@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"serialization@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"iomanager@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"dfmessages@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"detdataformats@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"trgdataformats@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"detchannelmaps@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"daqdataformats@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"readoutlibs@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"readoutmodules@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"dfmodules@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"hsilibs@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"timing@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"timinglibs@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"trigger@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"triggeralgs@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"trgtools@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"hdf5libs@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"oks@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"okssystem@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"oksutils@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"oksconfig@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"oksdbinterfaces@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"genconfig@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"coredal@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        depends_on(f"appdal@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=all build_type={build_type}")
        
        depends_on(f"daq-cmake@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=datautilities build_type={build_type}")
        depends_on(f"ers@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=datautilities build_type={build_type}")
        depends_on(f"logging@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=datautilities build_type={build_type}")
        depends_on(f"detdataformats@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=datautilities build_type={build_type}")
        depends_on(f"trgdataformats@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=datautilities build_type={build_type}")
        depends_on(f"daqdataformats@NB_DEV_240408_A9 build_type={build_type}", when=f"subset=datautilities build_type={build_type}")
