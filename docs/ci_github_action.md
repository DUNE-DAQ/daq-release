# Nightly Releases and Continuous Integration for DUNE DAQ Software



_JCF, Jan-6-2024: the CI dashboard below hasn't worked for some time. It would be good to reinstate a functioning dashboard showing each repo's build status_
## New Features

🆕 Checkout the [CI Dashboard](https://tiny.one/dunedaq-ci-dashboard) to get an overview of recently CI build status for DAQ repositories in `DUNE-DAQ` organization.

## Introduction

* [Continuous Integration](https://en.wikipedia.org/wiki/Continuous_integration) (CI):
    * developers integrate code into share repository frequently;
    * each integration will be verified by automated build and tests;
* CI configuration is easy in the case of one or a few packages; however, things can become quickly complicated when number of packages grows:
    * PRs for same feature affecting multiple packages need to be built together;
    * Each repo to have CI jobs building the whole release is just too expensive;
* Nightly release becomes necessary.
    * at the end of day, the develop release will be re-made;
    * it will be the base for each repo's CI in the next day;
    * releated PRs in multiple repos should be merged to develop on the same day.


## Nightly build DAQ releases

- As of Feb-11-2024, there are nightly builds both for the longer-term `v5`-focused development and for shorter-term `v4`-focused development used for data taking at `np04`
- Built off the `HEAD` of the `develop` or `production/v4` branches of the relevant DAQ repositories
- Builds in GitHub Actions every night, scheduled not to interfere with American or European working hours
- Published (deployed) to `/cvmfs/dunedaq-development.opensciencegrid.org`

### How the nightly releases are made

- The GitHub Workflows which perform the build-and-publish are defined in `.github/workflows/build-nightly-release-alma9.yml` and `build-nightly-release-sl7.yml`; note that they make use of `scripts/spack/build-release.sh` script to do so, which can also be run at the command line
- Analogous to how frozen and candidate releases are made, the nightly release's packages are defined via `release.yaml` files in `configs/coredaq/coredaq-develop`,`configs/fddaq/fddaq-develop` and `configs/nddaq/nddaq-develop`
- The Workflows will use the most recent versions of `daq-buildtools` and `daq-cmake` to build all the DUNE DAQ repos off of their `develop` or `production/v4` branches, and upload the build as artifacts ([an example](https://github.com/DUNE-DAQ/daq-release/actions/runs/7435650740)).
- The build is performed in an image which contains the externals packages and can be found in [the daq-docker repo](https://github.com/DUNE-DAQ/daq-docker); for the Alma9 build this image is `ghcr.io/dune-daq/alma9-slim-externals:v2.0` and for the SL7 build it's `ghcr.io/dune-daq/sl7-slim-externals:v1.1` 
- The resulting builds are saved in the images `ghcr.io/dune-daq/nightly-release-alma9:latest` (Alma9 build on Alma9 OS), `ghcr.io/dune-daq/nightly-release-sl7:latest` (SL7 build on SL7 OS) and `ghcr.io/dune-daq/nightly-release-c8:latest` (SL7 build on Centos Stream 8 OS)
- The workflow also publishes the builds automatically to `/cvmfs/dunedaq-development.opensciencegrid.org` 

_A snapshot of the [Actions page](https://github.com/DUNE-DAQ/daq-release/actions)_

![](https://i.imgur.com/gSITupq.png)

## Per-repository CI

- This is a build of the repository which runs inside `ghcr.io/dune-daq/nightly-release-alma9:latest`; i.e. the build is done against the latest nightly build

- CI build is triggered by:
    - Push to develop;
    - Pull request to develop;
    - Every night after the full nightly build is performed;
    - Manual trigger.

- Defined in the [.github repository](https://github.com/DUNE-DAQ/.github/blob/develop/workflow-templates/dunedaq-develop-cpp-ci.yml)

- Any change to this Workflow can be published to every individual repo using [this script](https://github.com/DUNE-DAQ/daq-release/blob/develop/scripts/github-ci/sync-ci-workflow-to-template.sh)


### Some details of the CI workflow

- It sets up a working directory using the develop release first;
- it *unsetup* the package itself before building;
- the build log and pre-build package is saved for 90 days on GitHub;
- You can add additional tests after the building step;
- Almost all DAQ packages currently included in the release have CI workflow enabled.


![](https://i.imgur.com/9VUQhsy.png)


### CI workflow triggered by Pull request

![](https://i.imgur.com/8LlUitg.png)

### CI workflow status for latest commit merged into develop

![](https://i.imgur.com/SO24De5.png)

### Manually trigger a CI workflow

![](https://i.imgur.com/vSm5vR6.png)

