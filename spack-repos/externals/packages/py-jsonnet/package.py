# Copyright 2013-2020 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

# JCF, Oct-1-2021: this is taken verbatim from the dunedaq-spack repo

from spack import *


class PyJsonnet(PythonPackage):
    """Python bindings to Jsonnet C library."""

    homepage = "https://jsonnet.org/ref/bindings.html"
    url      = "https://pypi.io/packages/source/j/jsonnet/jsonnet-0.16.0.tar.gz"

    maintainers = ['brettviren']

    version('0.16.0', sha256='4d6eff8c17e146dccd244eda45317577cd5e264ce8d5d0676f1f36afdc01307e')

    depends_on('py-setuptools', type='build')
