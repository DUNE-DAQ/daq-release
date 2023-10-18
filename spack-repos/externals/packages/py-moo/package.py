# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class PyMoo(PythonPackage):
    """Model oriented objects, tools for code generation."""

    homepage = "https://brettviren.github.io/moo"
    url      = "https://github.com/brettviren/moo/archive/0.6.3.tar.gz"
    git      = "https://github.com/brettviren/moo.git"

    maintainers = ['jcfreeman2']

    version('0.5.7', sha256='f881e396d521dbd1937d4e25949e80770dc8bad03e29888f63aa613bcce82222')
    version("0.6.7", sha256="f155c72e2d0f384fce384fa1c5b2fbab17e09aeb1bd4b10454a9b11d95eef8e2")
    version("0.6.4", sha256="e512bf088f62946fc9e75b82445c61169f976f230b0d3e918467052ea8982fb9")
    version("0.6.3", sha256="f8be51ed4e896bb66329d3d66185b4da375d0964f6a0e582d32565032eaad249")
    version("0.6.1", sha256="b100689a1469c7c1b0fa0d418e7391cb8a754fbe5706d0d7e455f5a1743e0ddf")


    depends_on('py-click', type=('build', 'run'))
    depends_on('py-jinja2', type=('build', 'run'))
    depends_on('py-jsonschema', type=('build', 'run'))
    depends_on('py-fastjsonschema', type=('build', 'run'))
    depends_on('py-jsonpointer', type=('build', 'run'))
    depends_on('py-numpy@1.23.0 ~blas ~lapack', type=('build', 'run'), when="@:0.6.4")
    depends_on('py-numpy@1.24.0 ~blas ~lapack', type=('build', 'run'), when="@0.6.7:")
    depends_on('py-openpyxl', type=('build', 'run'))

    # DUNE-specific
    depends_on('py-jsonnet', type=('build', 'run'))
    depends_on('py-anyconfig', type=('build', 'run'))
