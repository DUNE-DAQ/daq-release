# Creating a new DAQ release

## Overview

DAQ release tag follows the name convention of `vX.X.X` where X is a digial number; `develop` is used as the tag for the development release.


The steps of creating a new release include:

1. Tag relevant repos in [DUNE-DAQ](https://github.com/DUNE-DAQ) project with the new release tag (except the `develop` release, which by default takes the `develop` branch of each repo);
2. Prepare external packages and prebuilt DAQ packages for the new release, and publish them to the products area in cvmfs;
3. create a release manifest file, and run `make-release.py` to create:
  * umbrella packages;
  * directory structure of the release in cvmfs;
  * symbolic links to external packages and prebuilt DAQ pacakges in UPS products area;
  * a tarball for things created above which can be unrolled in the release path (e.g. `/cvmfs/dune.opensciencegrid.org/dunedaq/releases-tmp`);
4. publish the contents created by the last step to cvmfs;
5. submit an issue to update `quick-start.sh` if necessary.
