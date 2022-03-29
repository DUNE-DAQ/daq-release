# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class PyMoo(PythonPackage):
    """Model oriented objects, tools for code generation."""

    homepage = "https://brettviren.github.io/moo"
    url      = "https://github.com/brettviren/moo/archive/0.5.7.tar.gz"
    git      = "https://github.com/brettviren/moo.git"

    maintainers = ['jcfreeman2']

    version('0.5.7', sha256='f881e396d521dbd1937d4e25949e80770dc8bad03e29888f63aa613bcce82222')

    
    depends_on('py-click', type=('build', 'run'))
    depends_on('py-jinja2', type=('build', 'run'))
    depends_on('py-jsonschema', type=('build', 'run'))        
    depends_on('py-fastjsonschema', type=('build', 'run'))
    depends_on('py-jsonpointer', type=('build', 'run'))        
    depends_on('py-numpy@1.19.4 ~blas ~lapack', type=('build', 'run'))        
    depends_on('py-openpyxl', type=('build', 'run'))

    # DUNE-specific
    depends_on('py-jsonnet', type=('build', 'run'))
    depends_on('py-anyconfig', type=('build', 'run'))
