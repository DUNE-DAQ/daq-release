# daq-release

![GitHub issues](https://img.shields.io/github/issues/DUNE-DAQ/daq-release) ![GitHub pull requests](https://img.shields.io/github/issues-pr/DUNE-DAQ/daq-release) ![GitHub tag (latest by date)](https://img.shields.io/github/v/tag/DUNE-DAQ/daq-release?label=latest%20tag) ![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/DUNE-DAQ/daq-release)

This is a repo containing release manifest files of DUNE DAQ releases.

For a specific release (e.g. development release or a tagged release like
`v1.0.0`), the release manifest file is named as `release-vX_X_X.yaml` (
`release-development.yaml` for the development release.)

The parser script `bin/parse-manifest.py` in `daq-buildtools` package by default
parse the release manifest files from this repo first, and parse additional user
supplied manifest file. This repo contains a `user.yaml` file, which can be used
as a template for users to add additional setup or overwrite current setup for
the release.

Manifest files of a DAQ release:

* release-development.yaml
* release-vX_X_X.yaml
* release-v1_1_0.yaml
* user.yaml

Detailed explanations of the release manifest file:

```yaml
---
    # Release tag, in the format of vX.X.X or "development"
    # "vX.X.X" should be the actual tag used in other DAQ git repos.
    release: v1.0.0

    # List of pathes to UPS products, additional pathes in "user.yaml" will
    # be appended to the list.
    product_paths:
        - /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products

    # List of external dependencies; each element of the list is a dictionary
    # with keys in "name", "version", "variant"; if any of the field is empty,
    # supplied it with "~".
    #
    # Addtional elements in "user.yaml" will be appended to the list if the
    # its a new dependency, or overwrite the exisiting ones if it already exists.
    external_deps:
        - name: cmake
          version: v3_17_2
          variant: ~
        - name: gcc
          version: v8_2_0
          variant: ~
        - name: boost
          version: v1_70_0
          variant: "e19:prof"
        - name: cetlib
          version: v3_10_00
          variant: "e19:prof"
        - name: TRACE
          version: v3_15_09
          variant: "e19:prof"

    # List of DAQ's prebuilt packages in UPS products area; this is for DAQ
    # packages typically built by developers in their working directory.
    #
    # Similar rules of addition/overwriting for entries in "user.yaml" apply
    # as the section above.
    prebuilt_pkgs: ~

    # List of DAQ source packages to be cloned and built in users' working
    # directory. Each element is a dictionary as in "external_deps" seciton,
    # but each element has the keys of "name", "repo" and "tag" (or name of the
    # branch).
    #
    # Same packages can exist both in the "prebuilt_pkgs" section and the
    # "src_pkgs" section. In this case, the prebuilt packages will not be setup.
    # 
    # Similar rules apply to entries in "user.yaml".
    src_pkgs:
        - name: appfwk
          repo: https://github.com/DUNE-DAQ/appfwk.git
          tag: develop # tag or branch
        - name: ers
          repo: https://github.com/DUNE-DAQ/ers.git
          tag: ers-00-26-00

```
