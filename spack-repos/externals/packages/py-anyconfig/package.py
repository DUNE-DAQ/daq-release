# Copyright 2013-2020 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

# JCF, Oct-1-2021: this is taken verbatim from the dunedaq-spack repo

from spack import *


class PyAnyconfig(PythonPackage):
    """Common APIs to load and dump configuration files in various formats."""

    homepage = "https://github.com/ssato/python-anyconfig"
    url      = "https://pypi.io/packages/source/a/anyconfig/anyconfig-0.9.11.tar.gz"

    maintainers = ['brettviren']

    version("0.14.0", sha256="2cdf54af5dae8e91743ded82c54ed9d8aaefa3a9722f5d45e9b5f74b603e014d")
    version("0.13.0", sha256="03ff2e1762af388fbbbed1c1ab7f9f1ec006a91da7db3a68b963da6e7795d2ac")
    version("0.12.0", sha256="3091d7675740686fade85755537f2a7c6ccefa8659c5bb56174e02ded74b96d5")
    version("0.11.1", sha256="050dcade09799cd19c9b1f834e6df710ef824b388e6bb0af8c77be39b158b011")
    version("0.11.0", sha256="ec4eaad7250af23c98c86760954781361906b83027ae5b9d4da539e09765d308")
    version("0.10.1", sha256="f04a5490da8563c97fad15810b0debc92351dbd4b8058dfd82d32a30a41e7e5c")
    version("0.10.0", sha256="d9a25625aa72dd870e0712eac3e70fee50eb243af14b2f0cc6f305ec9191cacf")
    version("0.9.11", sha256="8888130cde5461cb39379afdd1d09b1b1342356210f0a6743a4b60f9973226f8")

    depends_on('py-setuptools', type='build')
