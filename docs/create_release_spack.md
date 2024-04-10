# Making a new frozen DAQ release

## Overview

Making a new frozen DAQ release consists of:

1. Preparations before performing each candidate/frozen release build, detailed below
2. Build/deploy candidate releases during the testing period of a release cycle
3. One last build of the release at the end of the testing period

A patch release is built in the same way as a non-patch release, except it doesn't involve candidate releases.

## Prepare for the new release

### Create release configurations

The release configuration package versions are defined by `configs/coredaq/coredaq-vX.Y.Z/release.yaml` (core release) and either `configs/fddaq/fddaq-vX.Y.Z/release.yaml` or `configs/nddaq/nddaq-vX.Y.Z/release.yaml` (detector release). To prepare the release configuration for a new release, it is best to start from (copy) the configuration of the develop release (`configs/coredaq/coredaq-develop/release.yaml` and `configs/fddaq/fddaq-develop/release.yaml` or `configs/nddaq/nddaq-develop/release.yaml`).

The release YAML file contains sections meant to define the versions of packages. In general, the `externals`, `devtools`, `systems` and `pymodules` sections will already have versions defined since you copied the YAML files from the develop release. While the develop release builds DUNE DAQ packages from the head of their `develop` branches, it uses versioned non-DUNE DAQ packages (e.g., Boost). However, you'll want to add the correct versions for the DUNE DAQ packages, e.g. edit
```
        - name: fddetdataformats
          version: "develop"
          commit: null
```
to
```
        - name: fddetdataformats
          version: v1.0.2
          commit: null
```
...where the `commit` field will be automatically calculated by the `make-release-repo.py` script run during the GitHub Actions which build releases. 

In addition to the `release.yaml` file, there also needs to be a `dbt-build-order.cmake` file in the `configs/fddaq-vX.Y.Z/` or `configs/nddaq-vX.Y.Z/` directory you created. Copy it from the corresponding `fddaq-develop`/`nddaq-develop` subdirectory. Update this file when there's a package being added/removed, or if the dependency tree changed and a package needs to be built earlier.


### Checks before doing test builds

It's worth to do several checks before starting any test builds. These checks include:

* Check version tags match with the version numbers listed in `CMakeLists.txt`; The script `scripts/checkout-daq-package.py` in this repo can help here. `python3 scripts/checkout-daq-package.py -i <path-to-release-config-yaml> -a -c -o /tmp` will checkout all the DAQ packages used in the release into `/tmp` and verify if the version tags match cmake version numbers
* Update Spack recipe files for DAQ packages with newly added dependencies; note that in general this will have happened already given that the nightly forms the basis of the candidate release
* (Optional) Check if developers got their dependencies in `CMakeLists.txt` to match those in `cmake/<pkgname>Config.cmake.in` files; see [this section](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#installing-your-project-as-a-local-package) of the daq-cmake instructions for more


## Building candidate releases

* Once the release configuration is ready, one can start the CI build for candidate releases. Go to the "Actions" tab of `daq-release` repo on GitHub. From the list of workflows on the left, select either "Alma9 build v5 candidate release" (for a develop release) or "Alma9 build v4 production candidate release" (for a production release), then click the "run workflow" button. A drop-down menu will show up. Put in the version of the base release in the `vX.Y.Z` format, the version of the detector release, the detector type for the release (`fd` or `nd`) and the candidate release number (start with 1, count up with later candidate releases). Click "Run workflow" to start the build. 
* Once the build is completed successfully, verify if the same version tags shown in the GitHub Action log match those in the tag collector spreadsheet
* To publish the candidate release to cvmfs:
    * Log in to `oasiscfs01.fnal.gov` as `cvmfsdunedaqdev`
    * Get the [`publish_release_to_cvmfs.sh` script](https://github.com/DUNE-DAQ/daq-release/blob/develop/scripts/cvmfs/publish_release_to_cvmfs.sh) (`git clone` this repo or use `curl`, e.g.)
    * Run the script without arguments for instructions; in a nutshell, it will publish the most recent release of a given specification (e.g., the most recent Alma9 near detector candidate build)
    * Run it with the desired specifications (e.g. `publish_release_to_cvmfs.sh candidate nd alma9 develop`)
    * After running the script, the release will take ~20 minutes before it appears on cvmfs
* After the candidate release is deployed and available on cvmfs, do the following simple tests:
    * Set up a work area based on the candidate release
    * If it's a far detector release, clone and build `daqsystemtest` used in this release and run `minimal_system_quick_test.py` in its `integtest` sub-directory
    * The above tests should be run on at least one NP04 DAQ server, and one Fermilab server
* Repeat all the above steps with "SL7" replacing "Alma9". 

## Building the frozen release

* The release will be cut at the end of the testing period. The build of the final frozen release can be done in a similar way as the candidate releases. Choose "Build frozen release" in the workflows list, and trigger the build by specifying release name used in `configs` and the number (starts from 1, increment it if second deployment to cvmfs is needed).
* Deploying the frozen release to cvmfs is the same as for a candidate release  _except_ you want to log in to `oasiscfs01.fnal.gov` as `cvmfsdunedaq` instead of `cvmfsdunedaqdev` and of course you'll want to pass `frozen` rather than `candidate` to the publishing script
* Do similar tests as shown in the section above for candidate releases
* If there is a new version of `daq-buildtools` for the release, it will need to be deployed to cvmfs too. Otherwise, creating a symbolic link in cvmfs to the latest tagged version will be sufficient. How to do this is described in the [documentation on cvmfs](publish_to_cvmfs.md).
* After the frozen release is rolled out, there will be remaining prep release and patch branches used in the production of the release. The software coordination team and the release coordinator should get in touch to establish if anything should be kept out of the merge to `develop`. The software coordination team will do the merge across all relevant repositories. Developers should handle any partial merge (cherry-pick).
* The last step of making a frozen release is to create release tags for all packages used by the release. To do so, use the script `scripts/create-release-tag.py`:
    * make sure `daq-release` is tagged for the new release, and the version is updated in the release YAML file. It will be tagged by the `create-release-tag.py` script;
    * `scripts/create-release-tag.py -h` to show the usage of the script;
    * `scripts/create-release-tag.py -a -t <release-tag> -i <release YAML file>` to tag all packages used by the release;
    * `scripts/create-release-tag.py -p <package> -r <ref_tag_or_branch_or_commit>` to tag a single package using specified ref;
    * `scripts/create-release-tag.py -p <package> -i <release YAML file>` to tag a single package using ref found in in release YAML file;
    * `-d` to delete release tags if found, `-f` to recreate release tags.
