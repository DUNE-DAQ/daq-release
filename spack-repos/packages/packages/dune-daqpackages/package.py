# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class DuneDaqpackages(BundlePackage):
    """A dummy package meant to pull in all the packages in the DUNE DAQ suite"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("develop")
    version("N22-02-20")
    version("dunedaq-v2.9.0")
    version("dunedaq-v2.8.2")
    version("dunedaq-v2.8.0")

    variant('build_type', default='RelWithDebInfo',
            description='The build type to build',
            values=('Debug', 'Release', 'RelWithDebInfo'),
            multi=True)

    depends_on("externals@dunedaq-v2.9.0", when="@dunedaq-v2.9.0")
    depends_on("externals@dunedaq-v2.8.2", when="@dunedaq-v2.8.2")
    depends_on("externals@develop", when="@develop")
    depends_on("externals@develop", when="@N22-02-20")

    for build_type in ["Debug", "RelWithDebInfo", "Release"]:

        for pkg in [ \
                     "appfwk", \
                     "cmdlib", \
                     "daq-cmake", \
                     "daqconf", \
                     "daqdataformats", \
                     "detchannelmaps", \
                     "detdataformats", \
                     "dfmessages", \
                     "dfmodules", \
                     "dqm", \
                     "ers", \
                     "erskafka", \
                     "fdreadoutlibs", \
                     "flxlibs", \
                     "hdf5libs", \
                     "influxopmon", \
                     "ipm", \
                     "kafkaopmon", \
                     "lbrulibs", \
                     "listrev", \
                     "logging", \
                     "ndreadoutlibs", \
                     "networkmanager", \
                     "nwqueueadapters", \
                     "opmonlib", \
                     "rcif", \
                     "readoutlibs", \
                     "readoutmodules", \
                     "restcmd", \
                     "serialization", \
                     "sspmodules", \
                     "timing", \
                     "timinglibs", \
                     "trigemu", \
                     "trigger", \
                     "triggeralgs", \
                     "utilities", \
                     "wibmod" \
        ]:
            depends_on(f'{pkg}@develop build_type={build_type}', when=f'@develop build_type={build_type}')
            depends_on(f'{pkg}@develop build_type={build_type}', when=f'@N22-02-20 build_type={build_type}')


        depends_on(f"daq-cmake@2.1.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"ers@1.1.5 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"logging@1.0.5 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"cmdlib@1.1.5 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"restcmd@1.1.4 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"opmonlib@1.3.4 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"rcif@1.1.1 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"appfwk@2.3.4 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"listrev@2.1.5 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"utilities@1.0.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"erskafka@1.3.3 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"kafkaopmon@1.3.2 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"influxopmon@1.5.3 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"ipm@2.3.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"serialization@1.2.3 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"dfmessages@2.2.2 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"detdataformats@3.2.1 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"detchannelmaps@1.0.3 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"daqdataformats@3.2.1 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"readoutlibs@1.0.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"readoutmodules@1.0.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"fdreadoutlibs@1.0.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"ndreadoutlibs@1.0.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"dfmodules@2.4.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"flxlibs@1.3.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"lbrulibs@1.0.6 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"networkmanager@1.0.2 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"nwqueueadapters@1.5.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"timing@6.0.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"timinglibs@1.5.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"trigemu@2.3.2 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"minidaqapp@4.2.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"trigger@1.2.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"triggeralgs@0.3.3 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"dqm@1.2.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"hdf5libs@1.0.3 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"wibmod@1.2.5 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")
        depends_on(f"sspmodules@1.1.0 build_type={build_type}", when=f"@dunedaq-v2.9.0 build_type={build_type}")

        depends_on(f'minidaqapp@4.1.3 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'dqm@1.1.6 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'flxlibs@1.2.3 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'readout@1.4.5 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'trigger@1.1.3 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'triggeralgs@0.3.1 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'timing@5.7.0 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'timinglibs@1.4.0 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'dfmodules@2.3.2 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'daqdataformats@3.2.1 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'detdataformats@3.2.1 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'detchannelmaps@1.0.2 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'dfmessages@2.2.1 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'listrev@2.1.4 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'ipm@2.2.0 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'nwqueueadapters@1.4.0 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'serialization@1.2.2 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'appfwk@2.3.3 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'rcif@1.1.1 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'cmdlib@1.1.4 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'opmonlib@1.3.4 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'logging@1.0.3 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'ers@1.1.3 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'daq-cmake@2.1.0 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'restcmd@1.1.3 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'erskafka@1.3.0 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'trigemu@2.3.1 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'erses@1.0.0 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'influxopmon@1.5.2 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'lbrulibs@1.0.5 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'kafkaopmon@1.3.0 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'wibmod@1.2.3 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'sspmodules@1.0.2 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')
        depends_on(f'hdf5libs@1.0.1 build_type={build_type}', when=f'@dunedaq-v2.8.2 build_type={build_type}')



        depends_on(f'minidaqapp@4.0.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'dfmodules@2.2.1 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'dqm@1.0.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'flxlibs@1.2.1 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'readout@1.4.2 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'trigger@1.1.2 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'timinglibs@1.2.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'triggeralgs@0.3.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'timing@5.5.1 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'dfmessages@2.2.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'nwqueueadapters@1.4.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'ipm@2.2.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'serialization@1.2.2 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'dataformats@3.0.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'appfwk@2.3.2 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'rcif@1.1.1 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'cmdlib@1.1.4 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'opmonlib@1.3.2 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'logging@1.0.3 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'ers@1.1.3 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'daq-cmake@develop build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'trace@3.16.02 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'listrev@2.1.4 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'restcmd@1.1.3 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'erskafka@1.3.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'trigemu@2.3.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'erses@1.0.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'influxopmon@1.4.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'lbrulibs@1.0.3 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')
        depends_on(f'kafkaopmon@1.3.0 build_type={build_type}', when=f'@dunedaq-v2.8.0 build_type={build_type}')


