# Copyright 2013-2020 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

# JCF, Oct-1-2021: this is taken verbatim from the dunedaq-spack repo

from spack import *


class PyAnyconfig(PythonPackage):
    """Common APIs to load and dump configuration files in various formats."""

    homepage = "https://github.com/ssato/python-anyconfig"
    url      = "https://pypi.io/packages/source/a/anyconfig/anyconfig-0.9.11.tar.gz"

    maintainers = ['brettviren']

    version('0.9.11', sha256='8888130cde5461cb39379afdd1d09b1b1342356210f0a6743a4b60f9973226f8')

    depends_on('py-setuptools', type='build')
