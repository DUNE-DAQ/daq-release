# daq-release

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
    release: v1.0.0

    product_paths:
        - /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products

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

    prebuilt_pkgs: ~

    src_pkgs:
        - name: appfwk
          repo: https://github.com/DUNE-DAQ/appfwk.git
          tag: develop # tag or branch
        - name: ers
          repo: https://github.com/DUNE-DAQ/ers.git
          tag: ers-00-26-00

```
