# Steps to build UPS products

## Set up build environment

1. Checkout the `daq-release` package;
  `git clone https://github.com/DUNE-DAQ/daq-release.git`
2. Under `daq-release/scripts/ups_build_scripts`, you will find subdirectories of package names which contains the version of the package and corresponding build script, together with its ups table file;
3. Create a new working directory, and copy all the contents under `daq-release/scripts/ups_build_scripts` to the working directory;
4. set up the build environment by running `source $WORK_DIR/setup`


## Obtain prebuilt external dependencies from SciSoft

Script [get_scisoft_pkgs.sh](https://github.com/DUNE-DAQ/daq-release/blob/master/scripts/ups_build_scripts/get_scisoft_pkgs.sh) is provided in this repo under `scripts/ups_build_scripts`.

This script contains a list of packages and their corresponding URLs of prebuilt tarballs on SciSoft's web server. The script will retrieve all the tarballs listed and unroll them in the current directory.

Before building DAQ's own UPS packages, it's recommended to run this script first. One may not need all the listed packages if the goal is to build a specific package or one type of variant only.


## Build additional external dependencies

At this step, the build script and its corresponding UPS table file should already exist in your working directory. To build each of the additional dependencies, one should run each of the build script one by one, such as:

```shell
pushd $WORK_DIR/ers/v00_26_00c && ./build_ers.sh $WORK_DIR e19 prof tar && popd
```

## Publish tarballs to cvmfs

Follow the [cvmfs publishing guide](publish_to_cvmfs.md) to publish the tarball to DUNE DAQ's cvmfs repo.
