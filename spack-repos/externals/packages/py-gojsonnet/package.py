# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

# ----------------------------------------------------------------------------
# If you submit this package back to Spack as a pull request,
# please first remove this boilerplate and all FIXME comments.
#
# This is a template package file for Spack.  We've put "FIXME"
# next to all the things you'll want to change. Once you've handled
# them, you can save this file and test your package like this:
#
#     spack install py-gojsonnet
#
# You can edit this file again by typing:
#
#     spack edit py-gojsonnet
#
# See the Spack documentation for more information on packaging.
# ----------------------------------------------------------------------------

from spack.package import *


class PyGojsonnet(PythonPackage):
    """Spack-ification of the Python bindings to Google's gojsonnet"""

    homepage = "https://pypi.org/project/gojsonnet/"
    pypi = "gojsonnet/gojsonnet-0.20.0.tar.gz"

    maintainers("jcfreeman2")

    version("0.20.0", sha256="9aede3b5734dee1c99dbec75dee3b086baaae92bd262d93f9217e21bf19c9682")

    depends_on('py-setuptools', type='build')
