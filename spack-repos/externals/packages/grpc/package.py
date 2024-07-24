# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
from spack.package import *


class Grpc(CMakePackage):
    """A high performance, open-source universal RPC framework."""

    maintainers("nazavode")

    homepage = "https://grpc.io"
    url = "https://github.com/grpc/grpc/archive/v1.39.0.tar.gz"

    version("1.65.1", sha256="b40840208c904d1364c1942d966474a2fdf1481f9708547d2d4c58812b8d9603")
    version("1.65.0", sha256="ebc3acfde70cfae3f4f04b8dbb72259540cb1dc427be362569fbc2607dabfe39")
    version("1.64.2", sha256="c682fc39baefc6e804d735e6b48141157b7213602cc66dbe0bf375b904d8b5f9")
    version("1.64.1", sha256="c5ad277fc21d4899f0e23f6f0337d8a2190d3c66c57ca868378be7c7bfa59fec")
    version("1.64.0", sha256="d5509e40fb24f6390deeef8a88668124f4ec77d2ebb3b1a957b235a2f08b70c0")
    version("1.63.1", sha256="31198ecf6ec8a79b0c42a90441ddce858e5fae690c2e300d5d08a9f45d7136df")
    version("1.39.0", sha256="b16992aa1c949c10d5d5ce2a62f9d99fa7de77da2943e643fb66dcaf075826d6")
    version("1.57.0", sha256="8393767af531b2d0549a4c26cf8ba1f665b16c16fb6c9238a7755e45444881dd")
    version("1.56.2", sha256="931f07db9d48cff6a6007c1033ba6d691fe655bea2765444bc1ad974dfc840aa")
    version("1.56.1", sha256="cc3e039aedd7b76f59cf922215adc7c308347a662be1e5e26711ffbc7fd3ce48")
    version("1.56.0", sha256="e034992a0b464042021f6d440f2090acc2422c103a322b0844e3921ccea981dc")
    version("1.55.3", sha256="f17d7dac5d02247805670296523942456a2fccfbea247631342bc708ddd4a2e6")
    version("1.54.3", sha256="17e4e1b100657b88027721220cbfb694d86c4b807e9257eaf2fb2d273b41b1b1")
    version("1.53.2", sha256="927d18a1c527932c0ff3bb54857f1d24630fe64d7ff98936c789772fe03e8fc9")

    version("1.44.0", sha256="8c05641b9f91cbc92f51cc4a5b3a226788d7a63f20af4ca7aaca50d92cc94a0d")
    version("1.39.0", sha256="b16992aa1c949c10d5d5ce2a62f9d99fa7de77da2943e643fb66dcaf075826d6")
    version("1.38.1", sha256="f60e5b112913bf776a22c16a3053cc02cf55e60bf27a959fd54d7aaf8e2da6e8")
    version("1.38.0", sha256="abd9e52c69000f2c051761cfa1f12d52d8b7647b6c66828a91d462e796f2aede")
    version("1.37.1", sha256="acf247ec3a52edaee5dee28644a4e485c5e5badf46bdb24a80ca1d76cb8f1174")
    version("1.37.0", sha256="c2dc8e876ea12052d6dd16704492fd8921df8c6d38c70c4708da332cf116df22")
    version("1.36.4", sha256="8eb9d86649c4d4a7df790226df28f081b97a62bf12c5c5fe9b5d31a29cd6541a")
    version("1.36.3", sha256="bb6de0544adddd54662ba1c314eff974e84c955c39204a4a2b733ccd990354b7")
    version("1.33.1", sha256="58eaee5c0f1bd0b92ebe1fa0606ec8f14798500620e7444726afcaf65041cb63")
    version("1.32.0", sha256="f880ebeb2ccf0e47721526c10dd97469200e40b5f101a0d9774eb69efa0bd07a")
    version("1.31.0", sha256="1236514199d3deb111a6dd7f6092f67617cd2b147f7eda7adbafccea95de7381")
    version("1.30.0", sha256="419dba362eaf8f1d36849ceee17c3e2ff8ff12ac666b42d3ff02a164ebe090e9")
    version("1.29.1", sha256="0343e6dbde66e9a31c691f2f61e98d79f3584e03a11511fad3f10e3667832a45")
    version("1.29.0", sha256="c0a6b40a222e51bea5c53090e9e65de46aee2d84c7fa7638f09cb68c3331b983")
    version("1.28.2", sha256="4bec3edf82556b539f7e9f3d3801cba540e272af87293a3f4178504239bd111e")
    version("1.28.1", sha256="4cbce7f708917b6e58b631c24c59fe720acc8fef5f959df9a58cdf9558d0a79b")
    version("1.28.0", sha256="d6277f77e0bb922d3f6f56c0f93292bb4cfabfc3c92b31ee5ccea0e100303612")
    version("1.27.0", sha256="3ccc4e5ae8c1ce844456e39cc11f1c991a7da74396faabe83d779836ef449bce")
    version("1.26.0", sha256="2fcb7f1ab160d6fd3aaade64520be3e5446fc4c6fa7ba6581afdc4e26094bd81")
    version("1.25.0", sha256="ffbe61269160ea745e487f79b0fd06b6edd3d50c6d9123f053b5634737cf2f69")
    version("1.24.3", sha256="c84b3fa140fcd6cce79b3f9de6357c5733a0071e04ca4e65ba5f8d306f10f033")
    version("1.23.1", sha256="dd7da002b15641e4841f20a1f3eb1e359edb69d5ccf8ac64c362823b05f523d9")

    variant("shared", default=False, description="Build shared instead of static libraries")
    variant(
        "codegen",
        default=True,
        description="Builds code generation plugins for protobuf " "compiler (protoc)",
    )
    variant(
        "cxxstd",
        default="17",
        values=("11", "14", "17"),
        multi=False,
        description="Use the specified C++ standard when building.",
    )

    depends_on("protobuf")
    depends_on("openssl")
    depends_on("zlib")
    depends_on("c-ares")
    depends_on("abseil-cpp", when="@1.27:")
    depends_on("re2+pic", when="@1.33.1:")

    def cmake_args(self):
        args = [
            self.define_from_variant("BUILD_SHARED_LIBS", "shared"),
            self.define_from_variant("gRPC_BUILD_CODEGEN", "codegen"),
            self.define_from_variant("CMAKE_CXX_STANDARD", "cxxstd"),
            "-DgRPC_BUILD_CSHARP_EXT:Bool=OFF",
            "-DgRPC_INSTALL:Bool=ON",
            # Tell grpc to skip vendoring and look for deps via find_package:
            "-DgRPC_CARES_PROVIDER:String=package",
            "-DgRPC_ZLIB_PROVIDER:String=package",
            "-DgRPC_SSL_PROVIDER:String=package",
            "-DgRPC_PROTOBUF_PROVIDER:String=package",
            "-DgRPC_USE_PROTO_LITE:Bool=OFF",
            "-DgRPC_PROTOBUF_PACKAGE_TYPE:String=CONFIG",
            # Disable tests:
            "-DgRPC_BUILD_TESTS:BOOL=OFF",
            "-DgRPC_GFLAGS_PROVIDER:String=none",
            "-DgRPC_BENCHMARK_PROVIDER:String=none",
        ]
        if self.spec.satisfies("@1.27.0:"):
            args.append("-DgRPC_ABSL_PROVIDER:String=package")
        if self.spec.satisfies("@1.33.1:"):
            args.append("-DgRPC_RE2_PROVIDER:String=package")
        return args
