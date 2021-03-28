# Publish files to cvmfs

## Staging area for DUNE DAQ's cvmfs repo

The staging area for DUNE DAQ's cvmfs repo is under `/grid/fermiapp/products/dunedaq`. It belongs to the user `dunedaq` and can be accessed from any DUNE DAQ GPVM node. The staging area is a mirror of `/cvmfs/dune.opensciencegrid.org/dunedaq`. Files need to be put into this staging area first before publishing to cvmfs.

## Publishing to cvmfs

Once the files are under the staging area mentioned above, login to `oasiscfs02.fnal.gov` as `cvmfsdune`, and run `/home/cvmfsdune/bin/dunedaq-sync` to sync the cvmfs repo to the staging area. There is a `/home/cvmfsdune/bin/dunedaq-sync-delete` script which uses `--delete` option when syncing with `rsync`. Be careful when using this script since it may delete exisiting files from the cvmfs repo and potentially break existing workflows.

### Adding more top level directories to the repo

The DUNE cvmfs repo utilizes `.cvmfsdirtab` in the top directory of the repo to help generating `.cvmfscatalog` files for subdirectories. If you need to add a new directory which is not covered by exisiting enties in the file, you will need to modify `/home/cvmfsdune/bin/dune-split` with the new directory path, and run it once.

### Other ways to publish files

Another way to publish files to cvmfs is to use [Tarball Publishing](https://cvmfs.readthedocs.io/en/stable/cpt-repo.html#tarball-publishing). One would not need to use a staging area in this case.

```bash
cvmfs_server ingest --tar_file <tarball.tar> --base_dir <path/where/extract/> dune.opensciencegrid.org
```

### Deleting files

To delete files/directories, you can use `/home/cvmfsdune/bin/dune-rm` with relative path to the files in the repo. Alternatively, you can do this by hand:

```bash
# open a transction on the publisher node
cvmfs_server transaction dune.opensciencegrid.org

# do the deletetion on the publisher node
rm <path_to_files_dirs_for_deletion>

# publish the transaction
cvmfs_server publish dune.opensciencegrid.org
```

Alternatively, you can use the `ingest` subcommand to `cvmfs_server` for file/dir deletion.

```bash
cvmfs_server ingest --delete <path/to/delete> dune.opensciencegrid.org

```






 
