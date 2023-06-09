# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class PyFastjsonschema(PythonPackage):
    """Fast JSON schema for Python"""

    homepage = "https://horejsek.github.io/python-fastjsonschema/"
    pypi     = "fastjsonschema/fastjsonschema-2.14.5.tar.gz"
    git      = "https://github.com/horejsek/python-fastjsonschema.git"

    maintainers = ['jcfreeman2']

    version('2.14.5', sha256='afbc235655f06356e46caa80190512e4d9222abfaca856041be5a74c665fa094')

    
