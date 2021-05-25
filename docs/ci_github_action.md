# Nightly releases and Continous Integration for DUNE DAQ Software

## New Features

🆕 Checkout the [CI Dashboard](https://fnal.zoom.us/my/alessandro.thea?pwd=WTdXUm9XZ1lWaHVuSWdlUXNBYk5XUT09) to get an overview of recently CI build status for DAQ repositories in `DUNE-DAQ` organization.

## Introduction

* Continous Integration:
    * developers integrate code into share repository frequently;
    * each integration will be verified by automated build and tests;
* CI configuration is easy in the case of one or a few packages; however, things can become quickly complicated when number of packages grows:
    * PRs for same feature affecting multiple packages need to be built together;
    * Each repo to have CI jobs building the whole release is just too expensive;
* Nightly release becomes necessary.
    * at the end of day, the develop release will be re-made;
    * it will be the base for each repo's CI in the next day;
    * releated PRs in multiple repos should be merged to develop on the same day.


## Nightly build DAQ release

- Contains the HEAD of develop branch across all DAQ repositories;
- builds on GitHub cloud node at 2:00am (CDT);
- build log and pre-built release (if the build succeeds) is saved for 90 days on GitHub;
- the release name is in the form of `NYY-MM-DD`;
- the release will be deployed to `/cvmfs/dunedaq-development.opensciencegrid.org`;
- a soft link named `last_successful` will ponit to the lastest nightly release.
- usage:
  - Just as a typical frozen release, 
  - use `-r` with the patth to the `nightly` directory in cvmfs
```sh
dbt-create.sh –r /cvmfs/dunedaq-development.opensciencegrid.org/nightly <NYY-MM-DD> workdir
```

### How the nightly releases are made

- The nightly release is defined under `daq-release/configs/dunedaq-develop`, just as a frozen DAQ release:
    - `release_manifest.sh` lists the external packages, DAQ packages and their versions;
- A daily cronjob running on a server with cvmfs access will assemble a Docker image:
    - based on `dunedaq/sl7-minimal:latest` image;
    - a layer of disk mirror containing external packages from cvmfs;
    - a layer of pre-built develop release;
    - the image is tagged as `dunedaq/sl7-minimal:dev`;
    - as well as a unique tag `dunedaq/sl7-minimal-nightly:NYY-MM-DD`.
- A GitHub Action workflow in the `daq-release` repo:
    - is configured to run one hour after the docker image was built, which will
    - checkout the develop branch of `daq-release` and `daq-buildtools`;
    - setup a working directory with only external packages from `dunedaq-develop` release;
    - checkout develop branches of all DAQ packages and build the release;
    - upload the pre-built release as ”artifacts” of the CI job.

---

### Related Docker images

- GitHub Repo: [daq-docker](https://github.com/DUNE-DAQ/daq-docker)
    - `sl7-minimal` contains the `Dockerfile` for making `dunedaq/sl7-minimal:latest` image;
    - any updates to the repo will trigger dockerhub to rebuild this image;
    - `sl7-ci` contains the `Dockerfile` and scripts to make the `dunedaq/sl7-minimal:dev` image;
    - it runs by a cronjob on a Fermilab server with access to cvmfs.
- Dockerhub image: [dunedaq/sl7-minimal](https://hub.docker.com/repository/docker/dunedaq/sl7-minimal)
- Dockerhub image: [dunedaq/sl7-minimal-nightly](https://hub.docker.com/repository/docker/dunedaq/sl7-minimal-nightly)


### Nightly GitHub Action for `daq-release`

- Workflow status page: [Nightly workflow](https://github.com/DUNE-DAQ/daq-release/actions)
- [Workflow configuration file](https://github.com/DUNE-DAQ/daq-release/blob/develop/.github/workflows/nightly.yml)
- [Latest workflow run](https://github.com/DUNE-DAQ/daq-release/actions/runs/762687473)

![](https://i.imgur.com/gSITupq.png)

## CI for DAQ packages

- Runs inside `dunedaq/sl7-minimal:dev` docker container;
- Use the latest develop/nightly release;
- Only build individual DAQ package;
- CI build is triggered by:
    - Push to develop;
    - Pull request to develop;
    - Daily at 2:00am (CDT);
    - Manual trigger.

### Instructions of setting CI workflow for a DAQ package

- Go to the `Actions` tab of the GitHub repo;
- Click "set up this workflow" using the template `dunedaq-develop CI for C++`
- Commit the workflow YAML file without change into the directory of `REPO/.github/workflows`
- A second template which rebuilds the whole release is also available.

![](https://i.imgur.com/EzkAojZ.png)


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

