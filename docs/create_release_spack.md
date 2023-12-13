# Making a new spack-based DAQ release

## Overview

Making a new DAQ release consists of:

1. Preparations before testing the build:
   * Create release configurations once all tags have been collected;
   * Check version tags with version numbers used in `CMakeLists.txt`;
   * Check if dependencies in `CMakeLists.txt` matches those in `cmake/<pkgname>Config.cmake.in` files;
   * Update spack recipe files with new dependencies;
2. Build/deploy candidate releases during the testing period of a release cycle;
3. One last build of the release at the end of the testing period.

Patch release is built in the same way as a regular release. It contains both phases of the candiadte then frozen releases.

## Prepare for the new release

### Create release configurations

The release configuration lives under `daq-release/configs/<release-name>`. To prepare the release configuration for a new release, it is best to start from the configuration of the develop release under `daq-release/configs/dunedaq-develop`.

Only the following two files are needed for a new spack-based release.

* `dbt-build-order.cmake`, update this file when there's a package being added/removed, or if the dependency tree changed and a package needs to be built earlier.
* `dunedaq-<vX.Y.Z>.yaml` (can be cloned from `dunedaq-develop.yaml` under `daq-release/configs/dunedaq-develop`).

The release YAML file `dunedaq-<vX.Y.Z>.yaml` contains the following sections:

* field name: `release_name`, data type: string, example: `dunedaq-develop`, `dunedaq-v3.1.0`, `rc-dunedaq-v3.1.0-1`
* field name: `externals`, data type: list; description: each element in the list is for one external package with information of package name, version, and variant.
* field name: `devtools`, data type: list; description: similar to `externals`, each element of the list is for one package
* field name: `systems`, data type: list; description: similar to `externals`, each element of the list is for one package
* field name: `dunedaq`, data type: list; description: each element is for one DAQ package, with its name, version/tag, and commit hash. Commit hash can be left as `null`. The commit hash will be captured at the time of building the release using the tag. 
* field name: `pymodules`, data type: list; description: each element is for one python module, with information of module name, source site (`pypi.org` or on GitHub).

Update the file with the new versions (and, if needed, new packages) in the release. Please note that if you need to update or add a Python package (i.e., an entry under `pymodules`) then you'll need to do a little preparation by updating `/cvmfs/dunedaq.opensciencegrid.org/pypi-repo`. How do to that is described [here](add_modules_to_pypi_repo.md). 

### Checks before doing test builds

It's worth to do several checks before starting any test builds. These checks include:

* Check version tags with version numbers used in `CMakeLists.txt`; The script `scripts/checkout-daq-package.py` in this repo can help here. `python3 scripts/checkout-daq-package.py -i <path-to-release-config-yaml> -a -c -o /tmp` will checkout all the DAQ packages used in the release into `/tmp` and verify if the version tags match cmake version numbers;
* Check if dependencies in `CMakeLists.txt` matches those in `cmake/<pkgname>Config.cmake.in` files;
* Update spack recipe files for DAQ packages with newly added dependencies.

## Building candidate releases

* Once the release configuration is ready, one can start the CI build for candidate releases. The build can be started via GitHub API or webUI. Go to the "Actions" tab of `daq-release` repo on GitHub, and select "Build candidate release" in the list of workflows in the workflows tab, click the "run workflow" button. A drop-down menu will show up. Put in the release name for the release cycle (e..g `dunedaq-v3.2.2` for locating the release configuration in the repo) and the candidate release name for this build (e.g. `rc-v3.2.2-1`, `-1` indicates the first build of the candidate release. **Note that there is a limit of 127 characters in spack's path names. `rc-dunedaq-v3.2.2-1` combined with cvmfs path is likely too long. Omit `dunedaq` in this case**), and then click "Run workflow" to start the build. The release name need to have less than 12 characters to make some spack path within its limit of characters.
* Once the build is completed successfully, verify if the same version tags shown in the CI log match those in the tag collector spreadsheet;
* Login to `cvmfsdunedaqdev@oasiscfs01.fnal.gov` and run `~/pull_and_publish_candidate_release.sh` to pull down the candidate release from GitHub and publish it to cvmfs.
* After the candidate release is deployed and available on cvmfs, do the following simple tests:
  * Set up a work area based on the candidate release;
  * Clone and build `dfmodules` used in this release, run `minimal_system_quick_test.py` in its `integtest` sub-directory.
  * The above tests should be run on at least one NP04 DAQ server, and one Fermilab server.

## Building the frozen release

* The release will be cut at the end of the testing period. The build of the final frozen release can be done in a similar way as the candidate releases. Choose "Build frozen release" in the workflows list, and trigger the build by specifying release name used in `daq-release/configs` and the build number (starts from 1, increment it if second deployment to cvmfs is needed).
* To deploy the release, login to `cvmfsdunedaq@oasiscfs01.fnal.gov` and run `~/pull_and_publish_frozen_release.sh`.  Note that the user is `cvmfsdunedaq` instead of `cvmfsdunedaqdev` (for the `dunedaq-development.opensicencgrid.org` repo).
* Do similar tests as shown in the section above for candidate releases;
* If there is a new version of `daq-buildtools` for the release, it will need to be deployed to cvmfs too. Otherwise, creating a symbolic link in cvmfs to the latest tagged version will be sufficient. 
To do so, login to `cvmfsdunedaq@oasiscfs01.fnal.gov` as `cvmfsdunedaq`, then do:
  1. `REPO=dunedaq.opensciencegrid.org; cvmfs_server transaction $REPO`;
  2. change directory to `/cvmfs/dunedaq.opensciencegrid.org/tools/dbt/`;
  3. (if needed) download the latest tagged version of dbt if it is not deployed yet, and expand it in the directory, rename the directory name using the tag;
  4. (if needed) move the `latest` link to the latest tag in the directory;
  5. create a symbolic link using the release tag, and point it to the latest tag in the directory;
  6. change to $HOME directory, and run `REPO=dunedaq.opensciencegrid.org; cvmfs_server publish $REPO` to publish the changes. (Note: it is important to not have open file descriptors under /cvmfs/ when publishing, thus one would need to change directories to somewhere outside of cvmfs before issuing the publishing command).

* After the frozen release is rolled out, regarding the `prep-release/dunedaq-vX.Y.Z` or `patch/dunedaq-vX.Y.Z` branches, the release coordinator should notify the software coordination team if anything should be kept out of the merge to develop. The software coordination team will do the merge across all relevant repositories. Developers should handle any partial merge (cherry-pick).

* The last step of making a frozen release is to create release tags for all packages used by the release. To do so, use the script `daq-release/scripts/create-release-tag.py`:
  * make sure `daq-release` is tagged for the new release, and the version is updated in the release YAML file. It will be tagged by the `create-release-tag.py` script;
  * `daq-release/scripts/create-release-tag.py -h` to show the usage of the script;
  * `daq-release/scripts/create-release-tag.py -a -t <release-tag> -i <release YAML file>` to tag all packages used by the release;
  * `daq-release/scripts/create-release-tag.py -p <package> -r <ref_tag_or_branch_or_commit>` to tag a single package using specified ref;
  * `daq-release/scripts/create-release-tag.py -p <package> -i <release YAML file>` to tag a single package using ref found in in release YAML file;
  * `-d` to delete release tags if found, `-f` to recreate release tags.
