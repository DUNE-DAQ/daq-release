# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Dunedaq(BundlePackage):
    """A dummy package meant to pull in all the packages in the DUNE DAQ suite"""

    homepage = "XHOMEPAGEX"

    version("XRELEASEX")

    variant('build_type', default='RelWithDebInfo',
            description='The build type to build',
            values=('Debug', 'Release', 'RelWithDebInfo'),
            multi=True)

    depends_on("externals@XRELEASEX", when="@XRELEASEX")

    def setup_run_environment(self, env):
        env.set('DUNE_DAQ_BASE_RELEASE', "XRELEASEX")

