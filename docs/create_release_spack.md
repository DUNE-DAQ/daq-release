# Making a new spack-based DAQ release

## Overview

Making a new DAQ release consists of:
1. adding release configurations once all tags have been collected;
2. building/deploying candidate releases during the testing period of a release cycle;
3. One last build of the release at the end of the testing period.


## Release configuration

The release configuration lives under `daq-release/configs/<release-name>`. To prepare the release configuration for a new release, it is best to start from the configuration of the develop release under `daq-release/configs/dunedaq-develop`. 

Only the following two files are needed for a new spack-based release.

* `dbt-build-order.cmake`, update this file when there's a package being added/removed, or if the dependency tree changed and a package needs to be built earlier.
* `dunedaq-<vX.Y.Z>.yaml` (can be cloned from `dunedaq-develop.yaml` under `daq-release/configs/dunedaq-develop`).

The release YAML file `dunedaq-<vX.Y.Z>.yaml` contains the following sections:

- field name: `release_name`, data type: string, example: `dunedaq-develop`, `dunedaq-v3.1.0`, `rc-dunedaq-v3.1.0-1`
- field name: `externals`, data type: list; description: each element in the list is for one external package with information of package name, version, and variant.
- field name: `devtools`, data type: list; description: similar to `externals`, each element of the list is for one package
- field name: `systems`, data type: list; description: similar to `externals`, each element of the list is for one package
- field name: `dunedaq`, data type: list; description: each element is for one DAQ package, with its name, version/tag, and commit hash. Commit hash can be left as `null`. The commit hash will be captured at the time of building the release using the tag. 
- field name: `pymodules`, data type: list; description: each element is for one python module, with information of module name, source site (`pypi.org` or on GitHub).

## Building candidate releases

Once the release configuration is ready, one can start the CI build for candidate releases. The build can be started via GitHub API or webUI. Go to the "Actions" tab of `daq-release` repo on GitHub, and select "Build candidate release" in the list of workflows in the workflows tab, click the "run workflow" button. A drop-down menu will show up. Put in the release name for the release cycle (for locating the release configuration in the repo) and the candidate release name for this build, and then click "Run workflow" to start the build.

Once the build is completed successfully, login to `cvmfsdunedaqdev@oasiscfs01.fnal.gov` and run `~/pull_and_publish_candidate_release.sh` to pull down the candidate release from GitHub and publish it to cvmfs.


## Building the release before cut-off

The release will be cut at the end of the testing period. The build of the final frozen release can be done in a similar way as the candidate releases. Choose "Build frozen release" in the workflows list, and trigger the build by specifying release name used in `daq-release/configs` and the frozen release name (in most cases, these two fields should have the same value.)

To deploy the release, login to `cvmfsdunedaq@oasiscfs01.fnal.gov` and run `~/pull_and_publish_frozen_release.sh`.  Note that the user is `cvmfsdunedaq` instead of `cvmfsdunedaqdev` (for the `dunedaq-development.opensicencgrid.org` repo).
