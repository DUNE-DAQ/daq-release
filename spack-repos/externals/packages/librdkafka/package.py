# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *

import os

class Librdkafka(AutotoolsPackage):
    """librdkafka is a C library implementation of the Apache Kafka
    protocol."""

    homepage = "https://github.com/edenhill/librdkafka"
    url      = "https://codeload.github.com/edenhill/librdkafka/tar.gz/refs/tags/v1.7.0"

    # JCF, Jan-7-2025
    # librdkafka 2.2.0 under consideration for externals v2.2
    version("2.2.0", sha256="af9a820cbecbc64115629471df7c7cecd40403b6c34bfdbb9223152677a47226", extension='tar.gz')

    # JCF, Oct-22-2021
    # dunedaq-v2.8.0 version of librdkafka is 1.7.0
    version('1.7.0', sha256='c71b8c5ff419da80c31bb8d3036a408c87ad523e0c7588e7660ee5f3c8973057', extension='tar.gz')

    depends_on('zstd')
    depends_on('lz4')
    #depends_on('openssl')

    # JCF, Jan-13-2025
    # librdkafka's shared object libraries link against SASL, so use a Spack-installed one rather than a system one to avoid link warnings (or worse)

    depends_on("cyrus-sasl")

    def patch(self):
        os.symlink(self.prefix + "/lib", self.prefix + "/lib64")

        if self.spec.satisfies('@1.7.0'):
            copy(join_path(os.path.dirname(__file__),
                           "RdKafkaConfig.cmake.v1.7.0"), self.prefix + "/RdKafkaConfig.cmake")
            copy(join_path(os.path.dirname(__file__),
                           "RdKafkaConfigVersion.cmake.v1.7.0"), self.prefix + "/RdKafkaConfigVersion.cmake")
            copy(join_path(os.path.dirname(__file__),
                           "RdKafkaTargets.cmake.v1.7.0"), self.prefix + "/RdKafkaTargets.cmake")
            copy(join_path(os.path.dirname(__file__),
                           "RdKafkaTargets-noconfig.cmake.v1.7.0"), self.prefix + "/RdKafkaTargets-noconfig.cmake")
        elif self.spec.satisfies('@2.2.0'):
            copy(join_path(os.path.dirname(__file__),
                           "RdKafkaConfig.cmake.v2.2.0"), self.prefix + "/RdKafkaConfig.cmake")
            copy(join_path(os.path.dirname(__file__),
                           "RdKafkaConfigVersion.cmake.v2.2.0"), self.prefix + "/RdKafkaConfigVersion.cmake")
            copy(join_path(os.path.dirname(__file__),
                           "RdKafkaTargets.cmake.v2.2.0"), self.prefix + "/RdKafkaTargets.cmake")
            copy(join_path(os.path.dirname(__file__),
                           "RdKafkaTargets-noconfig.cmake.v2.2.0"), self.prefix + "/RdKafkaTargets-noconfig.cmake")






