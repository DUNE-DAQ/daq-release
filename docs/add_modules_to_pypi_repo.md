

# Adding new modules to the Pypi-repo

## Using Github Action workflow to add new python modules

A workflow is available in `daq-release` repository to automatically add new python modules to DAQ's pypi-repo. It uses the `daq-release/configs/dunedaq-develop/dunedaq-develop.yaml` file to get the latest list of modules/versions.

To add a new module, or update a module's version, firstly update the `dunedaq-develop.yaml` file with the new module or version. Then go to "Actions" and trigger the "Update pypi-repo" workflow.

Once the workflow build is completed, you can login to `oasiscfs01.fnal.gov` as `cvmfsdunedaq` and run `~/cron_update_pypi_repo.sh`. This script will pull down a tarball from the recent "Update pypi-repo" workflow run, and publish changes to cvmfs.

A cron job is also in place to do the two steps above daily.

## Manually adding new modules (not recommended anymore)

If you have `pip2pi` installed, you can simply do the following to add any modules from `pypi` or tarballs:

1. `pip2pi $PATH_TO_PYPI_REPO <pypi_module_name==version>`;
    * `$PATH_TO_PYPI_REPO/<new_module_name>.whl` will be added after this;
    * This is the recommended way to install any packages from pypi.org as `pip2pi` will handle the dependencies automatically;
3. For tarballs, you can simply copy them over to the `pypi-repo` directory (NOTE: this is to be used only on our self-brewed packages);
    * Remember to rename the tarball to `module-name_X.X.X.tar.gz` before the next step;
4. Run `dir2pi $PATH_TO_PYPI_REPO` to generate new index files:
    * `$PATH_TO_PYPI_REPO/simple/index.html` will have a new entry for the newly-added module;
    * A new subdirectory will be created as `$PATH_TO_PYPI_REPO/simple/<new module name>` which contains:
      * An `index.html`
      * symbolic links to the wheel or tarball files under `$PATH_TO_PYPI_REPO` for each version of this module.

### Installing `pip2pi`

If you do not have `pip2pi` available, you can do the following to install it. Note that the latest official version (more than 2 years old) of `pip2pi` does not work well with python3. We have a patched version available [here](https://github.com/dingp/pip2pi/archive/1.0.0.tar.gz).

1. setup python3, skip this step if you already have access to it:
   * `source /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products/setup; setup python v3_8_3b`;
2. create and activate a virtual environment:
   * `python -m venv pypi-repo-venv`
   * `source pypi-repo-venv/bin/activate`
3. install `pip2py`:
   * `pip install https://github.com/dingp/pip2pi/archive/1.0.0.tar.gz`

Now you should have access to `pip2pi` and `dir2pi` commands. Next time, you can simply activate the virtual env for accessing these tools.


### Preapre the repo in staging area on "mwts"

1. The staging area including that for `pypi-repo` is under `/home/dingpf/cvmfs_dunedaq`;
2. Run `docker run --rm -it -v /home/dingpf/cvmfs_dunedaq:/cvmfs/dunedaq.opensciencegrid.org -v $PWD:/scratch dunedaq/sl7` to start a container;
3. Follow the instructions above to install `pip2pi`;
4. Follow the instruction above to add new packages to the repo.

### Publishing to cvmfs repo

1. Login as `cvmfsdunedaq` to `oasiscfs01.fnal.gov`;
2. Obtain a valid FNAL Kerberos ticket;
3. run `~/bin/dunedaq-sync pypi-repo` to publish the staging area on `docker-bd.fnal.gov`.
