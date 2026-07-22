# Copyright 2013-2024 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class PyPybind11Stubgen(PythonPackage):
    """Generates stubs for pybind11-wrapped python modules"""

    homepage = "https://github.com/sizmailov/pybind11-stubgen"
    url = "https://github.com/sizmailov/pybind11-stubgen/archive/refs/tags/v2.5.5.tar.gz"

    version("2.5.5", sha256="6d6bc411a953504fe930dde5003e6384c341a9cde7bb7e9d277aae3c71d93da2")

    depends_on("py-setuptools", type="build")
