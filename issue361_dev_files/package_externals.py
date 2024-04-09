# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Externals(BundlePackage):
    """A dummy package meant to pull in packages needed by DUNE DAQ developers"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("NB_DEV_240408_A9")

    # Generate from release YAML file
    depends_on("devtools@NB_DEV_240408_A9")
    # Additional dependencies defined in YAML file to be filled below

    depends_on("boost@1.77.0 +context+container cxxstd=2a")
    depends_on("trace@3.17.12")
    depends_on("nlohmann-json@3.9.0")
    depends_on("highfive@2.7.1 +mpi")
    depends_on("hdf5@1.12.0 +mpi+threadsafe")
    depends_on("py-pybind11@2.6.2")
    depends_on("protobuf@4.24.4")
    depends_on("krb5@1.19.2")
    depends_on("fmt@8.1.1 cxxstd=17 shared=True")
    depends_on("py-moo@0.6.7")
    depends_on("py-anyconfig@0.9.11")
    depends_on("py-jsonnet@0.16.0")
    depends_on("py-fastjsonschema@2.14.5")

