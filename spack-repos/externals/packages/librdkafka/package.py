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

    # JCF, Oct-22-2021
    # dunedaq-v2.8.0 version of librdkafka is 1.7.0
    version('1.7.0', sha256='c71b8c5ff419da80c31bb8d3036a408c87ad523e0c7588e7660ee5f3c8973057', extension='tar.gz')

    depends_on('zstd')
    depends_on('lz4')

    def patch(self):
        os.symlink(self.prefix + "/lib", self.prefix + "/lib64")

        copy(join_path(os.path.dirname(__file__),
             "RdKafkaConfig.cmake"), self.prefix + "/RdKafkaConfig.cmake")
        copy(join_path(os.path.dirname(__file__),
             "RdKafkaConfigVersion.cmake"), self.prefix + "/RdKafkaConfigVersion.cmake")
        copy(join_path(os.path.dirname(__file__),
             "RdKafkaTargets.cmake"), self.prefix + "/RdKafkaTargets.cmake")
        copy(join_path(os.path.dirname(__file__),
             "RdKafkaTargets-noconfig.cmake"), self.prefix + "/RdKafkaTargets-noconfig.cmake")
     
   



