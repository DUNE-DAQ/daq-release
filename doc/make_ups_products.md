# Steps to build UPS products

The instructions below applies only to systems running CentOS 7 or Scientific Linux 7. For other platforms, please exercise these instructions inside the [dunedaq/sl7](https://hub.docker.com/repository/docker/dunedaq/sl7) docker image using the `latest` tag (`dunedaq/sl7:latest`).

## Set up a working directory and the build environment

1. Checkout the `daq-release` package;
  `git clone https://github.com/DUNE-DAQ/daq-release.git`
2. Under `daq-release/scripts/ups_build_scripts`, you will find subdirectories of package names which contains the version of the package and corresponding build script, together with its ups table file;
3. Create a new working directory (`$WORK_DIR`), and copy all the contents under `daq-release/scripts/ups_build_scripts` to the working directory with `cp -rT daq-release/scripts/ups_build_scripts/ $WORK_DIR` (note the options used with the `cp` command to handle hidden directories `.upsfiles` and `.updfiles`);
4. set up the build environment by running `source $WORK_DIR/setup`


## Obtain prebuilt external dependencies from SciSoft

Script [get_scisoft_pkgs.sh](https://github.com/DUNE-DAQ/daq-release/blob/master/scripts/ups_build_scripts/get_scisoft_pkgs.sh) is provided in this repo under `scripts/ups_build_scripts`.

This script contains four lists of packages and their corresponding URLs of prebuilt tarballs on SciSoft's web server:
  * `PKGS_MINIMAL` is a minimal set of packages needed for build opt variant of UPS products of the DAQ develop release;
  * `PKGS_DEBUG` includes additional packages/variants needed for the debug build;
  * `PKGS_OLDER_VERSIONS` contains older versions of packages in DUNE DAQ's cvmfs repo;
  * `PKGS_NEWER_VERSIONS` contains packages/versions which have not been used by any release, but are currently in cvmfs for developers to play with.
By default, the script will retrieve packages listed in `PKGS_MINIMAL` and unpack them under the current directory.


Before building DAQ's own UPS packages, it's recommended to run this script first. (Note: one may not need all of the packages in `PKGS_MINIMAL` if the goal is to build one specific package).

## Build `folly` and its dependencies in `daq-externals` (more details to add in this section)

Follow instructions in [`daq-externals`](https://github.com/DUNE-DAQ/daq-externals) for building [folly](https://github.com/facebook/folly), and a set of its dependencies:
  * [double-conversion](https://github.com/google/double-conversion)
  * [fmt](https://github.com/fmtlib/fmt)
  * [glog](https://github.com/google/glog)
  * [googletest](https://github.com/google/googletest)
  * [libevent](https://github.com/libevent/libevent)

Put the built UPS packages under the working directory.

## Build additional external dependencies

At this step, the build script and its corresponding UPS table file should already exist in your working directory. To build each of the additional dependencies, one should run each of the build script one by one, such as:

```shell
pushd $WORK_DIR/ers/v0_26_00c && ./build_ers.sh $WORK_DIR e19 prof tar && popd
```

## Publish tarballs to cvmfs

Follow the [cvmfs publishing guide](publish_to_cvmfs.md) to publish the tarball to DUNE DAQ's cvmfs repo.
