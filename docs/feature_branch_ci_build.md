# Build a special nightly release with feature branches

A GitHub aciton named "Feature Branch Workflow" is available in the `daq-release` repository. The purpose of this workflow is to build the DAQ release using a specified feature branch in various DAQ packages wherever available, and using the `develop` branch if not.

This workflow is useful when there is a multi-package feature addition/change. It is important to have the same feature branch name used in those affected repositories. 

To trigger this workflow, go to the [Actions](https://github.com/DUNE-DAQ/daq-release/actions) tab of `daq-release` repo, and select 
`Feature Branch Workflow `on the left side. Or you can use [this link](https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-feature-branch.yml).
Click the "Run workflow" button with a drop-down menu. Fill in the `nightly tag prefix` and feature branch name, and click the green `Run workflow` button. 

The `nightly tag prefix` is a single letter which gets inserted after `N` in the normal nightly tag like `N<tag-prefix>YY-MM-DD`. This field only matters 
if the built release should be published to cvmfs. So it has a different nightly release tag than the usual nightly release.

## Publish the special nightly release to cvmfs

Upon success completion of the workflow, the special nightly release can be published to cvmfs. To do this, login to `oasiscfs01.fnal.gov` as `cvmfsdunedaqdev` and run `~/cron_pull_and_publish_spack_nightly_feature_release.sh`. Only SW coordination team members have the privillage to do so. Please contact them in the `daq-sw-librarians` slack channel.

## Enable daily CI builds

The special nightly release can be enabled as a daily build for a short period of time. This requires adding the `schedule` event trigger in the workflow, as that used in the spack nightly workflow. It also requires add a cron job on the cvmfs publisher node to run the publishing script as described in the section above.


![feature_branch_CI](https://user-images.githubusercontent.com/9438483/182927262-98b1f745-f6a1-4745-b433-ad6af6e12357.gif)
