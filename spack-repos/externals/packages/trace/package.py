# Copyright 2013-2020 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *
import os
from spack.util.environment import EnvironmentModifications

class Trace(CMakePackage):
    """A utility providing extremely fast print statements to memory (as well as stdout/stderr) developed by Ron Rechenmacher"""

    homepage = "https://github.com/art-daq/trace"
    git      = "https://github.com/art-daq/trace.git"

    version('3.16.02', commit='570d91139f7277966ae5259b0a11d85f8574e5dc')   # JCF, Sep-9-2021: v3_16_02 is used in the dunedaq-v2.8.0 suite
    version('3.17.03', commit='a1c3d7db117e8d965a181a73cedf4d4e45144025')   # DPF, May-10-2022: v3_17_03
    version('3.17.04', commit='526de4f948c7dbd32622b924c11d93836484e5d5')   # DPF, May-12-2022: v3_17_04
    version('3.17.06', commit='6245d834f68e57338cedd8af66dfb1b53582b7ad')   # DPF, Dec-16-2022: v3_17_06
    version('3.17.09', commit='d93a64f45e57bec87c0b890b9bbaaf94eb0b2e69')   # DPF, Mar-14-2023: v3_17_00
    version('3.17.11', commit='23f21ee9c53af5bd0e1b695038cda457517028fe')   # JCF, Mar-4-2024: v3_17_11

    patch('disable_cetmodules.diff', sha256='640816dfe077382bdb5345c8b5568c92e1ccb36274887e57241d826522ba5686', when='@3.16.02')
    patch('install-exec.diff', sha256='882deacf74407e1c83fb88ae8b2d16286751a49096682e441dc53175268ddcd9', when='@3.16.02')
    patch('install-scripts.diff', sha256='bac65a9fe92314596b18257583ad60cf0a5eab4baa34e7740fc16edde447bd9f', when='@3.16.02')
    patch('install-scripts-317.diff', sha256='18c57ad3d2b12a4f9b6425cf4939207c80cc5dd6', when='@3.17.03:3.17.04')
    patch('install-scripts-3176.diff', sha256='951e116e351e22219648d51da6d0cf5665fbe9153db9e3d443d65299ab8343be', when='@3.17.06')

    def setup_run_environment(self, env):
        env.prepend_path('PYTHONPATH', self.prefix.python)
        #trace_func_sh = os.path.join(self.prefix.bin, 'trace_functions.sh')
        #env.extend(EnvironmentModifications.from_sourcing_file(trace_func_sh, clean=True))

    #def setup_build_environment(self,env):
        #env.prepend_path('PYTHONPATH', self.prefix.python)
