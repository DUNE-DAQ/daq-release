# Making a new frozen DAQ release

## Overview

Making a new frozen DAQ release consists of:

1. Preparations before testing the build:
    * Set individual package version tags in release configurations once all tags have been collected
    * Crosscheck version tags with version numbers used in each repo's `CMakeLists.txt` file (this is partially automated; see below)
    * Check if listed dependencies in `CMakeLists.txt` match those in `cmake/<pkgname>Config.cmake.in` files
    * Check Spack `package.py` recipe files have proper dependencies 
2. Build/deploy candidate releases during the testing period of a release cycle
3. One last build of the release at the end of the testing period

A patch release is built in the same way as a non-patch release, except it doesn't involve candidate releases.

## Prepare for the new release

### Create release configurations

The release configuration lives under `configs/dunedaq-vX.Y.Z/release.yaml` (base release) and/or either `configs/fddaq-vX.Y.Z/release.yaml` and `configs/nddaq-vX.Y.Z/release.yaml` (detector release). To prepare the release configuration for a new release, it is best to start from (copy) the configuration of the develop release (`configs/dunedaq/dunedaq-develop/release.yaml`/`configs/fddaq/fddaq-develop/release.yaml`/`configs/nddaq/nddaq-develop/release.yaml`).

The release YAML file `dunedaq-<vX.Y.Z>.yaml` contains the following sections:

* field name: `release_name`, data type: string, example: `dunedaq-develop`, `dunedaq-v3.1.0`, `rc-dunedaq-v3.1.0-1`
* field name: `externals`, data type: list; description: each element in the list is for one external package with information of package name, version, and variant.
* field name: `devtools`, data type: list; description: similar to `externals`, each element of the list is for one package
* field name: `systems`, data type: list; description: similar to `externals`, each element of the list is for one package
* field name: `dunedaq`, data type: list; description: each element is for one DAQ package, with its name, version/tag, and commit hash. Commit hash can be left as `null`. The commit hash will be captured at the time of building the release using the tag. 
* field name: `pymodules`, data type: list; description: each element is for one python module, with information of module name, source site (`pypi.org` or on GitHub).

Update the file with the new versions (and, if needed, new packages) in the release.

In addition to the `release.yaml` file, there also needs to be a `dbt-build-order.cmake` file in the `configs/fddaq-vX.Y.Z/` or `configs/nddaq-vX.Y.Z/` directory you created. Copy it from the corresponding `fddaq-develop`/`nddaq-develop` subdirectory. Update this file when there's a package being added/removed, or if the dependency tree changed and a package needs to be built earlier.


### Checks before doing test builds

It's worth to do several checks before starting any test builds. These checks include:

* Check version tags with version numbers used in `CMakeLists.txt`; The script `scripts/checkout-daq-package.py` in this repo can help here. `python3 scripts/checkout-daq-package.py -i <path-to-release-config-yaml> -a -c -o /tmp` will checkout all the DAQ packages used in the release into `/tmp` and verify if the version tags match cmake version numbers;
* Check if dependencies in `CMakeLists.txt` matches those in `cmake/<pkgname>Config.cmake.in` files;
* Update Spack recipe files for DAQ packages with newly added dependencies.

## Building candidate releases

* Once the release configuration is ready, one can start the CI build for candidate releases. The build can be started via GitHub API or webUI. Go to the "Actions" tab of `daq-release` repo on GitHub, and select "Alma9 build candidate release" in the list of workflows in the workflows tab, then click the "run workflow" button. A drop-down menu will show up. Put in the release name for the release cycle in the `vX.Y.Z` format, the detector type for the release (`fd` or `nd`) and the candidate release build number (start with 1, count up with later candidate releases). Click "Run workflow" to start the build. 
* Once the build is completed successfully, verify if the same version tags shown in the CI log match those in the tag collector spreadsheet
* To publish the candidate release to cvmfs:
    * Log in to `oasiscfs01.fnal.gov` as `cvmfsdunedaqdev`
    * Get the [`publish_release_to_cvmfs.sh` script](https://github.com/DUNE-DAQ/daq-release/blob/develop/scripts/cvmfs/publish_release_to_cvmfs.sh) (`git clone` this repo or use `curl`, e.g.)
    * Run the script without arguments for instructions; in a nutshell, it will publish the most recent release of a given specification (e.g., the most recent Alma9 near detector candidate build)
    * Run it with the desired specifications (e.g. `publish_release_to_cvmfs.sh candidate nd alma9`)
    * After running the script, the release will take ~20 minutes before it appears on cvmfs
* After the candidate release is deployed and available on cvmfs, do the following simple tests:
    * Set up a work area based on the candidate release
    * If it's a far detector release, clone and build `daqsystemtest` used in this release and run `minimal_system_quick_test.py` in its `integtest` sub-directory
    * The above tests should be run on at least one NP04 DAQ server, and one Fermilab server
* Repeat all the above steps with "SL7" replacing "Alma9". _This will count as a new candidate build, so make sure the GitHub Action is given a different build number than the Alma9 build_.

## Building the frozen release

* The release will be cut at the end of the testing period. The build of the final frozen release can be done in a similar way as the candidate releases. Choose "Build frozen release" in the workflows list, and trigger the build by specifying release name used in `configs` and the build number (starts from 1, increment it if second deployment to cvmfs is needed).
* Deploying the frozen release to cvmfs is the same as for a candidate release  _except_ you want to log in to `oasiscfs01.fnal.gov` as `cvmfsdunedaq` instead of `cvmfsdunedaqdev` and of course you'll want to pass `frozen` rather than `candidate` to the publishing script
* Do similar tests as shown in the section above for candidate releases
* If there is a new version of `daq-buildtools` for the release, it will need to be deployed to cvmfs too. Otherwise, creating a symbolic link in cvmfs to the latest tagged version will be sufficient. 

* Adding a new daq-buildtools version

To do so, login to `cvmfsdunedaq@oasiscfs01.fnal.gov` as `cvmfsdunedaq`, then, [after setting up a cvmfs transaction](publish_to_cvmfs.md) for `dunedaq.opensciencegrid.org`, doing the following:
  1. change directory to `/cvmfs/dunedaq.opensciencegrid.org/tools/dbt/`;
  1. (if needed) download the latest tagged version of daq-buildtools if it is not deployed yet, and expand it in the directory, rename the directory name using the tag;
  1. (if needed) move the `latest` link to the latest tag in the directory;
  1. create a symbolic link using the release tag, and point it to the latest tag in the directory;
Then publish the change to cvmfs. 

* After the frozen release is rolled out, there will be remaining prep release and patch branches used in the production of the release. The software coordination team and the release coordinator should get in touch to establish if anything should be kept out of the merge to `develop`. The software coordination team will do the merge across all relevant repositories. Developers should handle any partial merge (cherry-pick).

* The last step of making a frozen release is to create release tags for all packages used by the release. To do so, use the script `scripts/create-release-tag.py`:
    * make sure `daq-release` is tagged for the new release, and the version is updated in the release YAML file. It will be tagged by the `create-release-tag.py` script;
    * `scripts/create-release-tag.py -h` to show the usage of the script;
    * `scripts/create-release-tag.py -a -t <release-tag> -i <release YAML file>` to tag all packages used by the release;
    * `scripts/create-release-tag.py -p <package> -r <ref_tag_or_branch_or_commit>` to tag a single package using specified ref;
    * `scripts/create-release-tag.py -p <package> -i <release YAML file>` to tag a single package using ref found in in release YAML file;
    * `-d` to delete release tags if found, `-f` to recreate release tags.
