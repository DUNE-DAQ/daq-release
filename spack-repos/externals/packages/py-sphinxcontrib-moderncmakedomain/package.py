# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class PySphinxcontribModerncmakedomain(PythonPackage):

    homepage = "https://github.com/slurps-mad-rips/moderncmakedomain"
    pypi = "sphinxcontrib-moderncmakedomain/sphinxcontrib-moderncmakedomain-3.19.tar.gz"



    version('3.19', sha256='b2900cc170b94ad53c59ae50a01961dc4c3ae9c12a8ec582d017b17abd69cea1')

    depends_on('py-sphinx')
