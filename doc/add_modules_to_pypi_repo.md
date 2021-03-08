# Adding new modules to the Pypi-repo

If you have `pip2pi` installed, you can simply do the following to add any modules from `pypi` or tarballs:

1. `pip2pi $PATH_TO_PYPI_REPO <pypi_module_name==version>`;
    * `$PATH_TO_PYPI_REPO/<new_module_name>.whl` will be added after this;
3. For tarballs, you can simply copy them over to the `pypi-repo` directory;
    * Remember to rename the tarball to `module-name_X.X.X.tar.gz` before the next step;
4. Run `dir2pi $PATH_TO_PYPI_REPO` to generate new index files:
    * `$PATH_TO_PYPI_REPO/simple/index.html` will have a new entry for the newly-added module;
    * A new subdirectory will be created as `$PATH_TO_PYPI_REPO/simple/<new module name>` which contains:
      * An `index.html`
      * symbolic links to the wheel or tarball files under `$PATH_TO_PYPI_REPO` for each version of this module.

## Installing `pip2pi`

If you do not have `pip2pi` available, you can do the following to install it. Note that the latest official version (more than 2 years old) of `pip2pi` does not work well with python3. We have a patched version available [here](https://github.com/dingp/pip2pi/archive/1.0.0.tar.gz).

1. setup python3, skip this step if you already have access to it:
   * `source /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products/setup; seutp python v3_8_3b`;
2. create and activate a virtual environment:
   * `python -m venv pypi-repo-venv`
   * `source pypi-repo-venv/bin/activate`
3. install `pip2py`:
   * `pip install https://github.com/dingp/pip2pi/archive/1.0.0.tar.gz`

Now you should have access to `pip2pi` and `dir2pi` commands. Next time, you can simply activate the virtual env for accessing these tools.
