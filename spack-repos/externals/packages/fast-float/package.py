# Copyright 2013-2024 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class FastFloat(CMakePackage):
    """JCF, Dec-9-2024: a package.py for https://github.com/fastfloat/fast_float, used by the (contemporary, 2024) folly package"""

    homepage = "https://github.com/fastfloat/fast_float"
    url = "https://github.com/fastfloat/fast_float/archive/refs/tags/v7.0.0.tar.gz"

    maintainers("jcfreeman2")

    # FIXME: Add the SPDX identifier of the project's license below.
    # See https://spdx.org/licenses/ for a list. Upon manually verifying
    # the license, set checked_by to your Github username.
    license("UNKNOWN", checked_by="github_user1")

    version("7.0.0", sha256="d2a08e722f461fe699ba61392cd29e6b23be013d0f56e50c7786d0954bffcb17")
    version("6.1.6", sha256="4458aae4b0eb55717968edda42987cabf5f7fc737aee8fede87a70035dba9ab0")
    version("6.1.5", sha256="597126ff5edc3ee59d502c210ded229401a30dafecb96a513135e9719fcad55f")
    version("6.1.4", sha256="12cb6d250824160ca16bcb9d51f0ca7693d0d10cb444f34f1093bc02acfce704")
    version("6.1.3", sha256="7dd99cc2ff44e07dc2a42bed0c6b8c4a8ee4e3b1c330f77073b6cfdb48724c8e")
    version("6.1.2", sha256="6163ae88b48eaaf900933de210b194cd2efd47bb394010bab256a4afef6b8d05")
    version("6.1.1", sha256="10159a4a58ba95fe9389c3c97fe7de9a543622aa0dcc12dd9356d755e9a94cb4")
    version("6.1.0", sha256="5a629e1f18f037ad0016c41ead630ea471cccbcdf60239ed3466c491d8e7c908")
    version("6.0.0", sha256="7e98671ef4cc7ed7f44b3b13f80156c8d2d9244fac55deace28bd05b0a2c7c8e")
    version("5.3.0", sha256="2f3bc50670455534dcaedc9dcd0517b71152f319d0cec8625f21c51d23eaf4b9")

    # FIXME: Add dependencies if required.
    # depends_on("foo")

    def cmake_args(self):
        # FIXME: Add arguments other than
        # FIXME: CMAKE_INSTALL_PREFIX and CMAKE_BUILD_TYPE
        # FIXME: If not needed delete this function
        args = []
        return args
