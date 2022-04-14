# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *

import os

class Cpr(CMakePackage):
    """C++ Requests: Curl for People, a spiritual port of Python Requests"""

    homepage = "https://docs.libcpr.org/"
    url      = "https://codeload.github.com/libcpr/cpr/legacy.tar.gz/1.5.2"

    maintainers = ['jcfreeman2']

    version('1.5.2', sha256='7ac7683e302f26801b8c097701bc6c0f346c8d5c49d1b947f92ea961fa447841', extension='tar.gz')

    depends_on("openssl")

    def patch(self):
        copy(join_path(os.path.dirname(__file__),
             "cpr-config.cmake"), self.prefix + "/cpr-config.cmake")

    def cmake_args(self):
        return ['-DBUILD_CPR_TESTS=OFF',
                '-DUSE_SYSTEM_CURL=ON']
