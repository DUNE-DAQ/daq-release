# About daq-release


[![Nightly Integration Test Workflow](https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-integtest.yml/badge.svg)](https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-integtest.yml)

[![AL9 Spack Nightly Workflow](https://github.com/DUNE-DAQ/daq-release/actions/workflows/build-nightly-release-alma9.yml/badge.svg)](https://github.com/DUNE-DAQ/daq-release/actions/workflows/build-nightly-release-alma9.yml)

[![Spack Nightly Workflow](https://github.com/DUNE-DAQ/daq-release/actions/workflows/build-nightly-release-sl7.yml/badge.svg)](https://github.com/DUNE-DAQ/daq-release/actions/workflows/build-nightly-release-sl7.yml)


This is a repo containing DUNE DAQ release making tools, configuration files, and build scripts for both DUNE-DAQ and external packages. 

## Table of contents

### For DAQ software developers and users:
1. [DAQ software development workflow](development_workflow_gitflow.md)
2. [List of GitHub Teams and Repositories](team_repos.md)
3. [How to build external software with Spack in a local workarea](Build-external-packages-with-spack-in-a-work-area.md)

### For the Software Coordination team (expert only):

1. [Nightly Releases and Continuous Integration](docs/ci_github_action.md)
2. [Creating a new DAQ release](create_release_spack.md)
3. [How to publish files to cvmfs](publish_to_cvmfs.md)
4. [How to build a new stack of external software](Build-new-external-software-stack.md)
