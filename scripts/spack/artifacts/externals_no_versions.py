# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Externals(BundlePackage):
    """A dummy package meant to pull in packages needed by DUNE DAQ developers"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("EXT2.2ADD")

    # Generate from release YAML file
    depends_on("devtools@EXT2.2ADD")
    # Additional dependencies defined in YAML file to be filled below

    depends_on("boost +context+container cxxstd=2a")
    depends_on("cetlib +lite")
    depends_on("trace")
    depends_on("nlohmann-json")
    depends_on("pistache@dunedaq-v2.8.0")
    depends_on("highfive +mpi")
    depends_on("hdf5 +mpi+threadsafe")
    depends_on("libarchive")
    depends_on("libzmq")
    depends_on("cppzmq")
    depends_on("msgpack-c")
    depends_on("py-pybind11")
    depends_on("uhal")
    depends_on("cpr")
    depends_on("librdkafka")
    depends_on("protobuf")
    depends_on("grpc cxxstd=17")
    depends_on("felix-software@dunedaq-v4.2.0")
    depends_on("folly cxxstd=2a")
    depends_on("fftw ~mpi")
    depends_on("cli11")
    depends_on("intel-tbb")
    depends_on("dpdk")
    depends_on("fmt cxxstd=17 shared=True")
    depends_on("py-moo@0.6.7")
    depends_on("py-anyconfig")
    depends_on("py-jsonnet")
    depends_on("py-fastjsonschema")
    depends_on("rclone")
    depends_on("libtorrent")
