# Publish files to cvmfs

## Staging area for DUNE DAQ's cvmfs repo

The staging area for DUNE DAQ's cvmfs repo is under `/grid/fermiapp/products/dunedaq`. It belongs to the user `dunedaq` and can be accessed from any DUNE DAQ GPVM node. The staging area contains mirrors of `/cvmfs/dunedaq.opensciencegrid.org` and `/cvmfs/dunedaq-development.opensciencegrid.org`. Files need to be put into this staging area first before publishing to cvmfs.

## Publishing to cvmfs

1. Once the files are under the staging area mentioned above, login to `oasiscfs01.fnal.gov` as `cvmfsdunedaq` if publishing to `/cvmfs/dunedaq.opensciencegrid.org` (or as `cvmfsdunedaqdev` if publishing to `/cvmfs/dunedaq-development.opensciencegrid.org`);
2. Run `~/bin/dunedaq-sync` (or `~/bin/dunedaq-dev-sync`) to sync the cvmfs repo to the staging area. There is a `~/dunedaq-sync-delete` script which uses `--delete` option when syncing with `rsync`. Be careful when using this script since it may delete exisiting files from the cvmfs repo and potentially break existing workflows.

### Adding more top level directories to the repo

The DUNE cvmfs repo utilizes `.cvmfsdirtab` in the top directory of the repo to help generating `.cvmfscatalog` files for subdirectories. If you need to add a new directory which is not covered by exisiting enties in the file, you will need to modify `~/bin/dune-split` with the new directory path, and run it once.

### Other ways to publish files

Another way to publish files to cvmfs is to use [Tarball Publishing](https://cvmfs.readthedocs.io/en/stable/cpt-repo.html#tarball-publishing). One would not need to use a staging area in this case.

```bash
cvmfs_server ingest --tar_file <tarball.tar> --base_dir <path/where/extract/> dune.opensciencegrid.org
```

Note that we recently experienced file permission issues with content published by `ingest` tarballs. It is recommended to always use the staging area method.

### Deleting files

You can do the following to delete files/directories:

```bash
# open a transction on the publisher node
cvmfs_server transaction dunedaq.opensciencegrid.org

# do the deletetion on the publisher node
rm <path_to_files_dirs_for_deletion>

# publish the transaction
cvmfs_server publish dunedaq.opensciencegrid.org
```

Alternatively, you can use the `ingest` subcommand to `cvmfs_server` for file/dir deletion.

```bash
cvmfs_server ingest --delete <path/to/delete> dunedaq.opensciencegrid.org

```






 
