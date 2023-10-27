# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class PyJsonschemaSpecifications(PythonPackage):
    """Spackification of jsonschema-specifications, the JSON Schema meta-schemas and vocabularies, exposed as a Registry"""

    homepage = "https://pypi.org/project/jsonschema-specifications/"
    pypi = "jsonschema_specifications/jsonschema_specifications-2023.7.1.tar.gz"

    maintainers("jcfreeman2")

    version("2023.7.1", sha256="c91a50404e88a1f6ba40636778e2ee08f6e24c5613fe4c53ac24578a5a7f72bb")

    depends_on("py-hatchling", type=("build", "run"))
    depends_on("py-hatch-vcs", type=("build", "run"))

    patch("jsonschema-specifications-remove-unrecognized-classifiers.patch")

