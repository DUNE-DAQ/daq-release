# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class PyRpdsPy(PythonPackage):
    """Spackification of rpds-py, Python bindings to Rust's persistent data structures"""

    homepage = "https://pypi.org/project/rpds-py/"
    pypi = "rpds_py/rpds_py-0.10.6.tar.gz"

    maintainers("jcfreeman2")

    version("0.10.6", sha256="4ce5a708d65a8dbf3748d2474b580d606b1b9f91b5c6ab2a316e0b0cf7a4ba50")

    depends_on("py-maturin", type=("build"))
