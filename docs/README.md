# About daq-release

[![Nightly Integration Test Workflow](https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-integtest.yml/badge.svg)](https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-integtest.yml)

[![Spack Nightly Workflow](https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-spack.yml/badge.svg)](https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-spack.yml)

[![AL9 Spack Nightly Workflow](https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-spack-alma9.yml/badge.svg)](https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-spack-alma9.yml)



This is a repo containing DUNE DAQ release making tools, configuarion files, and build scripts of external packages. 

## Table of content

### For DAQ software developers and users:
1. [DAQ software development workflow -- Gitflow](development_workflow_gitflow.md)
2. [List of GitHub Teams and Repositories](team_repos.md)
3. [How to build external software with spack in a local workarea](Build-external-packages-with-spack-in-a-work-area.md)
4. [To-be-updated: Continous Integraion for DUNE DAQ software](ci_github_action.md)

### For DAQ software coordiantion team:

1. [Creating a new DAQ release](create_release_spack.md)
2. [How to publish files to cvmfs](publish_to_cvmfs.md)
3. [How to add new modules to `pypi-repo`](add_modules_to_pypi_repo.md)
4. [How to build a new stack of external software](Build-new-external-software-stack.md)
