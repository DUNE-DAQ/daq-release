# Copyright 2013-2022 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class Czmq(AutotoolsPackage):
    """ A C interface to the ZMQ library """
    homepage = "http://czmq.zeromq.org"
    url      = "https://github.com/zeromq/czmq/archive/v4.0.2.tar.gz"

    version('4.2.1',     sha256='83457cd32a2c2615b8d7ebcf91b198cb0d8df383a2072b96835ab250164d8a83'),
    version('4.2.0',     sha256='cfab29c2b3cc8a845749758a51e1dd5f5160c1ef57e2a41ea96e4c2dcc8feceb')
    version('4.1.1',     sha256='f00ff419881dc2a05d0686c8467cd89b4882677fc56f31c0e2cc81c134cbb0c0')
    version('4.1.0',     sha256='3befa35b4886b5298e8329b4f0aa5bb9bde0e7439bd3c5c53295cb988371fc11')
    version('4.0.2',     sha256='808c7a2262ca733d7a2c362e0a00fdbe5ec517d90fa017ba405b7cdb4f81eb89')
    version('4.0.1',     sha256='0fc7294d983df7c2d6dc9b28ad7cd970377d25b33103aa82932bdb7fa6207215')
    version('4.0.0-rc1', sha256='c11070c7fa08a1eed6e7c13e4f5c6c97338bd3f1eb92ac0fac48629c2494ddfe')
    version('4.0.0',     sha256='9b9d0686fd9a7bdf67c381229c3743cafa94971606d9984fd7f8c4df1fd56fad')
    version('3.0.2',     sha256='8bca39ab69375fa4e981daf87b3feae85384d5b40cef6adbe9d5eb063357699a')
    version('3.0.1',     sha256='b8a13c7064956ccd01ee9ce5c952a2a859af70bf450389bc33b7e0e3c67b6b17')



    depends_on('libtool', type='build')
    depends_on('automake', type='build')
    depends_on('autoconf', type='build')
    depends_on('pkgconfig', type='build')
    depends_on('docbook-xml', type='build')
    depends_on('docbook-xsl', type='build')
    depends_on('uuid')
    depends_on('libzmq')

    def autoreconf(self, spec, prefix):
        autogen = Executable('./autogen.sh')
        autogen()

    def configure_args(self):
        config_args = []
        if 'clang' in self.compiler.name:
            config_args.append("CFLAGS=-Wno-gnu")
            config_args.append("CXXFLAS=-Wno-gnu")
        return config_args
