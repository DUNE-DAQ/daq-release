# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class PyReferencing(PythonPackage):
    """Spackification of the referencing package, an implementation-agnostic implementation of JSON reference resolution."""

    homepage = "https://pypi.org/project/referencing/"
    pypi = "referencing/referencing-0.30.2.tar.gz"

    maintainers("jcfreeman2")

    version("0.30.2", sha256="794ad8003c65938edcdbc027f1933215e0d0ccc0291e3ce20a4d87432b59efc0")

    depends_on("py-hatchling", type=("build", "run"))
    depends_on("py-hatch-vcs", type=("build", "run"))
    depends_on("py-rpds-py", type=("build", "run"))

    patch("referencing-remove-unrecognized-classifiers.patch")
