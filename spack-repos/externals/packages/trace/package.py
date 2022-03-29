# Copyright 2013-2020 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *
import os
from spack.util.environment import EnvironmentModifications

class Trace(CMakePackage):
    """FIXME: Put a proper description of your package here."""

    homepage = "https://github.com/art-daq/trace"
    git      = "https://github.com/art-daq/trace.git"

    version('stable', branch='stable')
    version('master', branch='master')
    version('3.16.02', commit='570d91139f7277966ae5259b0a11d85f8574e5dc')   # JCF, Sep-9-2021: v3_16_02 is used in the dunedaq-v2.8.0 suite
    version('3.15.09', commit='f429a6a8b52925c31678cab5643f67df16f06fd5')

    patch('disable_cetmodules.diff', sha256='640816dfe077382bdb5345c8b5568c92e1ccb36274887e57241d826522ba5686')
    patch('install-exec.diff', when='3.15.09')
    patch('install-scripts.diff', when='stable')

