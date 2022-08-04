# Build a special nightly release with feature branches

A GitHub aciton named "Feature Branch Workflow" is available in the `daq-release` repository. The purpose of this workflow is to build the DAQ release using a specified feature branch in various DAQ packages wherever available, and using the `develop` branch if not.

This workflow is useful when there is a multi-package feature addition/change. It is important to have the same feature branch name used in those affected repositories. 

To trigger this workflow, go to the [Actions](https://github.com/DUNE-DAQ/daq-release/actions) tab of `daq-release` repo, and select 
`Feature Branch Workflow `on the left side. Or you can use [this link](https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-feature-branch.yml).
Click the "Run workflow" button with a drop-down menu. Fill in the `nightly tag prefix` and feature branch name, and click the green `Run workflow` button. 

The `nightly tag prefix` is a single letter which gets inserted after `N` in the normal nightly tag like `N<tag-prefix>YY-MM-DD`. This field only matters 
if the built release should be published to cvmfs. So it has a different nightly release tag than the usual nightly release.




