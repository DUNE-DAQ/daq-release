# Copyright 2013-2024 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class CyrusSasl(AutotoolsPackage):
    """This is the Cyrus SASL API implementation. It can be used on the
    client or server side to provide authentication and authorization
    services."""

    homepage = "https://github.com/cyrusimap/cyrus-sasl"
    url = "https://github.com/cyrusimap/cyrus-sasl/archive/cyrus-sasl-2.1.27.tar.gz"
    git = "https://github.com/cyrusimap/cyrus-sasl.git"

    license("custom")

    version("2.1.28", sha256="3e38933a30b9ce183a5488b4f6a5937a702549cde0d3287903d80968ad4ec341")

    # JCF, Jan-7-2026: this patch is needed since gcc 14.x is less
    # forgiving than earlier gcc versions of the time.h header being
    # incorrectly left out

    patch("dunedaq-cyrus-fix-time-headers.patch", sha256="de9c4d2bd940ad6473bb5c3b407c4243e8c45d0fac375c46d7da4237178f0cc9", when="@2.1.28")

    # JCF, Jan-7-2026: Also, thanks to some obsolete tests during the
    # configuration phase, cyrus-sasl incorrectly thinks that it
    # doesn't have time.h available, so manually override this

    def setup_build_environment(self, env):
        env.append_flags('CPPFLAGS', '-DHAVE_TIME_H=1')    

    depends_on("m4", type="build")
    depends_on("autoconf", type="build")
    depends_on("automake", type="build")
    depends_on("libtool", type="build")
