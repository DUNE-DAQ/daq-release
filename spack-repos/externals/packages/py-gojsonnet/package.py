# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class PyGojsonnet(PythonPackage):
    """Spack-ification of the Python bindings to Google's gojsonnet"""

    homepage = "https://pypi.org/project/gojsonnet/"
    pypi = "gojsonnet/gojsonnet-0.20.0.tar.gz"

    maintainers("jcfreeman2")

    version("0.20.0", sha256="9aede3b5734dee1c99dbec75dee3b086baaae92bd262d93f9217e21bf19c9682")

    depends_on('py-setuptools', type='build')
    depends_on('go', type='build')
