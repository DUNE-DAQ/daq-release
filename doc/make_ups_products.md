# Steps to build UPS products

The instructions below applies only to systems running CentOS 7 or Scientific Linux 7. For other platforms, please exercise these instructions inside the [dunedaq/sl7](https://hub.docker.com/repository/docker/dunedaq/sl7) docker image using the `latest` tag (`dunedaq/sl7:latest`).


## Doing everything together

A bootstrap has been written for the current UPS products used by the develop release (as of 10/20/2020). It is named `bootstrap-ups-build.sh` under `daq-release/scripts`. Running this script in an empty directory will build all the currently used UPS products. The tarballs made by the build will be stored under `$PWD/tarballs` by default.

```shell
# change to am empty directory, then run the following
curl -O https://raw.githubusercontent.com/DUNE-DAQ/daq-release/master/scripts/bootstrap-ups-build.sh
chmod +x bootstrap-ups-build.sh
./bootstrap-ups-build.sh
```

Once finished running the script, your working directory will look like

```shell
# tree -L 2
.
├── bootstrap-ups-build.sh
├── build_pkgs_cet
│   ├── build_double_conversion
│   ├── build_fmt
│   ├── build_folly
│   ├── build_glog
│   ├── build_googletest
│   └── build_libevent
├── daq-externals
│   ├── cmake-externalproject
│   ├── README.md
│   └── ups
├── daq-release
│   ├── develop.yaml
│   ├── doc
│   ├── README.md
│   ├── release_develop.yaml
│   ├── release_v1-0-0.yaml
│   ├── release_v1-1-0.yaml
│   ├── scripts
│   ├── user_template.yaml
│   ├── user_workenv.yaml
│   └── v1-1-0.yaml
├── products
│   ├── boost
│   ├── cetbuildtools
│   ├── cetlib
│   ├── cetlib_except
│   ├── cetpkgsupport
│   ├── clang
│   ├── cmake
│   ├── cppunit
│   ├── double_conversion
│   ├── ers
│   ├── fmt
│   ├── folly
│   ├── gcc
│   ├── get_scisoft_pkgs.sh
│   ├── git
│   ├── glog
│   ├── googletest
│   ├── hdf5
│   ├── hep_concurrency
│   ├── highfive
│   ├── libevent
│   ├── ninja
│   ├── nlohmann_json
│   ├── pistache
│   ├── pistache-2020.10.07-sl7-x86_64-e19-prof.tar.bz2
│   ├── python
│   ├── pyyaml
│   ├── setup -> ups/v6_0_8/Linux64bit+3.10-2.17/ups/setup
│   ├── setups -> ups/v6_0_8/Linux64bit+3.10-2.17/ups/setups
│   ├── setups_layout
│   ├── sqlite
│   ├── ssibuildshims
│   ├── tbb
│   ├── TRACE
│   └── ups
└── tarballs
    ├── double_conversion-3.1.5-slf7-x86_64-e19-prof.tar.bz2
    ├── ers-0.26.00c-sl7-x86_64-e19-prof.tar.bz2
    ├── fmt-6.2.1-slf7-x86_64-e19-prof.tar.bz2
    ├── folly-2020.05.25-slf7-x86_64-e19-prof.tar.bz2
    ├── glog-0.4.0-slf7-x86_64-e19-prof.tar.bz2
    ├── googletest-1.8.1-slf7-x86_64-e19-prof.tar.bz2
    ├── highfive-2.2.2-sl7-x86_64-e19-prof.tar.bz2
    ├── libevent-2.1.8-slf7-x86_64-e19-prof.tar.bz2
    ├── nlohmann_json-3.9.0b-sl7-x86_64-e19-prof.tar.bz2
    └── pyyaml-5.3.1-sl7-x86_64-p383b.tar.bz2

45 directories, 25 files
```

## Step-by-step instructions

### Set up a working directory and the build environment

1. Checkout the `daq-release` package;
  `git clone https://github.com/DUNE-DAQ/daq-release.git`
2. Under `daq-release/scripts/ups_build_scripts`, you will find subdirectories of package names which contains the version of the package and corresponding build script, together with its ups table file;
3. Create a new working directory (`$WORK_DIR`), and copy all the contents under `daq-release/scripts/ups_build_scripts` to the working directory with `cp -rT daq-release/scripts/ups_build_scripts/ $WORK_DIR` (note the options used with the `cp` command to handle hidden directories `.upsfiles` and `.updfiles`);
4. set up the build environment by running `source $WORK_DIR/setup`


### Obtain prebuilt external dependencies from SciSoft

Script [get_scisoft_pkgs.sh](https://github.com/DUNE-DAQ/daq-release/blob/master/scripts/ups_build_scripts/get_scisoft_pkgs.sh) is provided in this repo under `scripts/ups_build_scripts`.

This script contains four lists of packages and their corresponding URLs of prebuilt tarballs on SciSoft's web server:
  * `PKGS_MINIMAL` is a minimal set of packages needed for build opt variant of UPS products of the DAQ develop release;
  * `PKGS_DEBUG` includes additional packages/variants needed for the debug build;
  * `PKGS_OLDER_VERSIONS` contains older versions of packages in DUNE DAQ's cvmfs repo;
  * `PKGS_NEWER_VERSIONS` contains packages/versions which have not been used by any release, but are currently in cvmfs for developers to play with.
By default, the script will retrieve packages listed in `PKGS_MINIMAL` and unpack them under the current directory.


Before building DAQ's own UPS packages, it's recommended to run this script first. (Note: one may not need all of the packages in `PKGS_MINIMAL` if the goal is to build one specific package).

### Build `folly` and its dependencies with `cetbuildtools`

`folly` and its dependencies are set up to be built into UPS packages with `cetbuildtools` using recipes written in [`daq-externals`](https://github.com/DUNE-DAQ/daq-externals) for building [folly](https://github.com/facebook/folly). The dependencies are:
  * [double-conversion](https://github.com/google/double-conversion)
  * [fmt](https://github.com/fmtlib/fmt)
  * [glog](https://github.com/google/glog)
  * [googletest](https://github.com/google/googletest)
  * [libevent](https://github.com/libevent/libevent)

Here are the steps to build these UPS products (using `fmt` as an example):

```shell
# create another working directory
mkdir -p $WORK_DIR/../daq-externals-workdir

# checkout daq-externals package
cd $WORK_DIR/../daq-externals-workdir
git clone https://github.com/DUNE-DAQ/daq-externals.git

# setup 
# To build one package, e.g. fmt:
mkdir $WORK_DIR/../daq-externals-workdir/build_fmt
export CETPKG_INSTALL=$WORK_DIR
source $WORK_DIR/../daq-externals-workdir/daq-externals/ups/multi-project/fmt/ups/setup_for_development -p e19
buildtool -A -c -l -bti -j8

# the UPS products will be installed under $WORK_DIR together with UPS products from SciSoft
# In addition, a tarball of the UPS product can be found in the build directory.
ls $WORK_DIR/../daq-externals-workdir/build_fmt/fmt-6.2.1-slf7-x86_64-e19-prof.tar.bz2 

```

Building other dependencies is a similar process as above. Once finished, one can continue with building `folly` using similar steps.

### Build additional external dependencies

At this step, the build script and its corresponding UPS table file should already exist in your working directory. To build each of the additional dependencies, one should run each of the build script one by one, such as:

```shell
pushd $WORK_DIR/ers/v0_26_00c && ./build_ers.sh $WORK_DIR e19 prof tar && popd
```

## Publish tarballs to cvmfs

You can follow the [cvmfs publishing guide](publish_to_cvmfs.md) to publish the tarball to DUNE DAQ's cvmfs repo.
