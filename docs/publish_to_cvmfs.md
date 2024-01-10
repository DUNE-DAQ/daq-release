# Publish files to cvmfs

We use CERN's [CernVM-FS filesystem](https://cernvm.cern.ch/fs/),
better known as "cvmfs", to make our software releases available to
the collaboration. A good user tutorial for how to publish files to it can be found [here](https://cvmfs-contrib.github.io/cvmfs-tutorial-2021/04_publishing/), but this document will focus specifically on DUNE DAQ Software Coordination-specific use of cvmfs.

We use two cvmfs repositories: `dunedaq.opensciencegrid.org` for our frozen releases, and `dunedaq-development.opensciencegrid.org` for our candidate and nightly releases. To alter the former, log in to `oasiscfs01.fnal.gov` as `cvmfsdunedaq`; to alter the latter, log in as `cvmfsdunedaqdev`. Once you've done that, you can use the `cvmfs_server` command (or a script which uses it) to modify cvmfs. 

Publishing the releases typically does not involve running `cvmfs_server` directly. [The nightly is published directly](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-release/ci_github_action/#how-the-nightly-releases-are-made) via the GitHub Workflow, and [candidate and frozen releases are published via the `publish_release_to_cvmfs.sh` script](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-release/create_release_spack/#building-candidate-releases)

The most common use case for directly calling `cvmfs_server` is when adding a new daq-buildtools version

A cheat sheet for using `cvmfs_server` which is even simpler than the tutorial linked to above is the following:
1. Open a cvmfs server transaction via `cvmfs_server transaction <name of cvmfs repo>`, where you'd use the name of one of our two cvmfs repos here
1. Copy files to `/cvmfs/<name of cvmfs repo>/desired/subdirectory` using standard Linux commands
1. Publish what you've done via `cvmfs_server publish <name of cvmfs repo>`. Note that you can't be in the `/cvmfs` directory tree when you execute this. Note also it takes ~20 minutes before your published changes actually show up for others on cvmfs
1. Before you execute `cvmfs_server publish ...`, if you wish to cancel your work you can execute `cvmfs_server abort <name of cvmfs repo>`

The cvmfs area is relied upon by everyone so of course it's important to treat it carefully. In the event that something bad happens, however, cvmfs provides a method to roll back unwanted changes. Each cvmfs publish has an associated tag; if you run `cvmfs_server tag -l <name of cvmfs repo>` you can see the tag for each publish in the first column of the output. If you wish to roll back to a particular tag of the cvmfs repo's existence, you can run `cvmfs_server rollback -t <name of tag> <name of cvmfs repo>`. In general this would only be under exceptional circumstances and in consultation with other members of software coordination. 

_JCF, Jan-10-2024: section below not yet updated_

## Publishing to cvmfs

1. Once the files are under the staging area mentioned above, login to `oasiscfs01.fnal.gov` as `cvmfsdunedaq` if publishing to `/cvmfs/dunedaq.opensciencegrid.org` (or as `cvmfsdunedaqdev` if publishing to `/cvmfs/dunedaq-development.opensciencegrid.org`);
2. Run `~/bin/dunedaq-sync` (or `~/bin/dunedaq-dev-sync`) to sync the cvmfs repo to the staging area. There is a `~/dunedaq-sync-delete` script which uses `--delete` option when syncing with `rsync`. Be careful when using this script since it may delete exisiting files from the cvmfs repo and potentially break existing workflows.

### Adding more top level directories to the repo

The DUNE cvmfs repo utilizes `.cvmfsdirtab` in the top directory of the repo to help generating `.cvmfscatalog` files for subdirectories. If you need to add a new directory which is not covered by exisiting enties in the file, you will need to modify `~/bin/dune-split` with the new directory path, and run it once.

### Publishing externals

_JCF, Jan-10-2024: to be added_






 
