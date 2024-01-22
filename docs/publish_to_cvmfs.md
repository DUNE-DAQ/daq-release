# Publish files to cvmfs

## Overview

We use CERN's [CernVM-FS filesystem](https://cernvm.cern.ch/fs/),
better known as "cvmfs", to make our software releases available to
the collaboration. A good user tutorial for how to publish files to it can be found [here](https://cvmfs-contrib.github.io/cvmfs-tutorial-2021/04_publishing/), but this page will focus on DUNE DAQ Software Coordination-specific use of cvmfs.

We use two cvmfs repositories: `dunedaq.opensciencegrid.org` for our frozen releases, and `dunedaq-development.opensciencegrid.org` for our candidate and nightly releases. To alter the former, log in to `oasiscfs01.fnal.gov` as `cvmfsdunedaq`; to alter the latter, log in as `cvmfsdunedaqdev`. Once you've done that, you can use the `cvmfs_server` command (or a script which uses it) to modify cvmfs. 

## The basics

A cheat sheet for using `cvmfs_server` which is even simpler than the tutorial linked to above is the following:
1. Open a cvmfs server transaction via `cvmfs_server transaction <name of cvmfs repo>`, where you'd use the name of one of our two cvmfs repos here
1. Copy files to `/cvmfs/<name of cvmfs repo>/desired/subdirectory` using standard Linux commands
1. Publish what you've done via `cvmfs_server publish <name of cvmfs repo>`. Note that you can't be in the `/cvmfs` directory tree when you execute this. Note also it takes ~20 minutes before your published changes actually show up for others on cvmfs
1. Before you execute `cvmfs_server publish ...`, if you wish to cancel your work you can execute `cvmfs_server abort <name of cvmfs repo>`

The cvmfs area is relied upon by everyone so of course it's important to treat it carefully. In the event that something bad happens, however, cvmfs provides a method to roll back unwanted changes. Each cvmfs publish has an associated tag; if you run `cvmfs_server tag -l <name of cvmfs repo>` you can see the tag for each publish in the first column of the output. If you wish to roll back to a particular tag of the cvmfs repo's existence, you can run `cvmfs_server rollback -t <name of tag> <name of cvmfs repo>`. In general this would only be under exceptional circumstances and in consultation with other members of software coordination. 

## Updating daq-buildtools on cvmfs

Most uses of `cvmfs_server` are wrapped within scripts, but adding a
new version of daq-buildtools to cvmfs is an exception; this section
also doubles as a concrete example of the commands in the previous
section. Log in to `oasiscfs01.fnal.gov` as `cvmfsdunedaq` and execute the following:

1. `cvmfs_server transaction dunedaq.opensciencegrid.org`
1. `cd /cvmfs/dunedaq.opensciencegrid.org/tools/dbt/`
1. `git clone https://github.com/DUNE-DAQ/daq-buildtools`
1. `rm -rf daq-buildtools/.git   # No need for git info on cvmfs`
1. `mv daq-buildtools <version>`
1. `rm latest`
1. `ln -s <version> latest`
1. `cd ~   # you can't run the next command from within /cvmfs`
1. `cvmfs_server publish dunedaq.opensciencegrid.org`

## Updating releases on cvmfs

Publishing the releases typically does not involve running `cvmfs_server` directly. [The nightly is published directly](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-release/ci_github_action/#how-the-nightly-releases-are-made) via GitHub Workflows, and [candidate and frozen releases are published via the `publish_release_to_cvmfs.sh` script](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-release/create_release_spack/#building-candidate-releases)

## Updating a particular directory on cvmfs

As a member of Software Coordination, if you have an account on `mwts.fnal.gov` with a staging area called `/home/<your username>/docker-scratch/cvmfs_dunedaq`, the `scripts/cvmfs/publish_directory_to_cvmfs.sh` script will synchronize a given subdirectory in your staging area to a given subdirectory on `/cvmfs`. E.g., if you've made changes to the v2.0 externals in your staging area, you can do the following if you've logged in to `oasiscfs01.fnal.gov` as `cvmfsdunedaq` and are in the base of a freshly-updated daq-release repo:
```
./scripts/cvmfs/publish_directory_to_cvmfs.sh spack/externals/ext-v2.0 
``` 
You'll be prompted a couple of times if you're sure you want to go ahead, since of course modifying `/cvmfs` is a sensitive expert action. _Be aware_ that this script behaves as a "clobber", so if along with adding files to your staging-area `spack/externals/ext-v2.0` directory you've also removed some files, the corresponding files on cvmfs will also be removed. 
