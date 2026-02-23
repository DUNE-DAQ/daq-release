# Creating A New Environment

## Intro

As of this writing (Feb-11-2026) there's effectively only one
environment (*) in DUNE DAQ, containing all packages needed for any
far detector development and operation. These are installed as nightly
builds whose names have the format `NFD_DEV_<YYMMDD>_A9` and candidate
and stable builds whose names contain `fddaq` (e.g.,
`fddaq-v5.5.0-a9`). Historically, there was also a near detector
environment, though for the time being that's gone by the
wayside. These instructions describe how to add a new environment.

## Environment model (conceptually)

Any environment follows the classic far detector environment
model. That is, it includes a Spack umbrella package whose purpose
is only to depend on other DUNE DAQ Spack-installed packages which,
unlike it, contain packages with actual code. These packages can be
either part of our core packages (reused by more than one environment,
e.g. `logging`) or they can be specific to the environment in question
(e.g., `fddetdataformats` for the far detector environment). The core
packages are depended on by an umbrella package, `coredaq`, which has
different variants depending on the environment in
question. E.g. `coredaq` with `subset=fddaq` will depend on data
readout packages such as `dpdklibs`, whereas a data-analysis focused
package likely would not. `coredaq` itself, along with depending on
core DUNE DAQ packages, will depend on an umbrella package called
`externals` which depends on the external packages which DUNE DAQ
packages need, and which itself has the same variant. So, we can
represent the Spack packages in the far detector environment as
follows:

```
fddaq ^coredaq subset=fddaq ^externals subset=fddaq
```

Just like the `subset` variant of `coredaq` controls which core
packages are loaded in an environment, the `subset` variant of
`externals` controls which external packages are in the
environment. E.g., `externals subset=fddaq` depends on `dpdk`, while
an `externals` with a subset variant for a data analysis-focused
environment shouldn't.

Last but not least, a DUNE DAQ software environment also has a Python `.venv` environment with pip-installed packages. Again, some Python packages
may be needed for some environments but not for others.

## Environment model (technically)

We can use the far detector example to examine how the above model is
implemented. Firstly, packages in the environment and their versions
are defined in YAML files. They can be found as follows from the base of daq-release:
* `configs/coredaq/<env name>-<build type>/release.yaml`: the core DUNE DAQ Spack-installed packages _and_ the Spack-installed external packages for the environment
* `configs/<env name>/<env name>-<build type>/release.yaml`: the non-core (target specific) DUNE DAQ Spack-installed packages _and_ the Python environment packages
* `configs/<env	name>/<env name>-<build type>/dbt-build-order.cmake`: the order in which packages will build in a software development work area based on the environment

Note that it's largely for historic reasons that the external packages
are defined alongside the core packages and the `pip`-installed Python
packages are defined alongside the non-core packages.

Additionally, the `package.py` files Spack uses to install the DUNE
DAQ packages are provided in `daq-release`. Those for the core
packages are available in
`spack-repos/coredaq-repo-template/packages`, while those those specific to an
environment are available in `spack-repos/<env
name>-repo-template/packages`

# Putting this all together and adding a new environment

We can break this down into a series of steps. In the steps below, for
convenience we'll pretend the new environment is called `yourenv`. The
first steps, based on the description in the previous section, are
fairly obvious:
1. Create a `configs/coredaq/yourenv-develop/` configuration directory, copy `release.yaml` from `configs/coredaq/coredaq-develop` and define what Spack-installed core DUNE DAQ packages and externals your environment needs by deleting any unnecessary packages. 
1. Similarly create a `configs/yourenv/yourenv-develop/` configuration directory, copy `release.yaml` and `dbt-build-order.cmake` over from `configs/fddaq/fddaq-develop`, and edit `release.yaml` to define which environment-specific Spack-installed DUNE DAQ packages and pip-installed DUNE DAQ Python packages go into the environment. Don't forget to change the references to `fddaq` in the copied file to the name of your new environment. 
1. Create `spack-repos/yourenv-repo-template`
1. Copy `spack-repos/fddaq-repo-template/repo.yaml` into it and again, replace the name `fddaq` in the file
1. Create a subdirectory, `spack-repos/yourenv-repo-template/packages`, which itself is to be populated with subdirectories containing `package.py`'s (`yourpkg1/package.py`, `yourpkg2/package.py`, etc.). This, of course, is analogous to the subdirectories in `spack-repos/fddaq-repo-template/packages`, but will contain the `package.py`s for the Spack-installed DUNE DAQ packages referred to in `configs/yourenv/yourenv-develop/release.yaml` rather than for `fddaq`. 
1. Create the subdirectories for the repos which have code (i.e., not the umbrella package, just yet) and create their `package.py` files. These `package.py` files will be processed by `make-release-repo.py` and hence you'll want to include tokens for that script to swap out (`XVERSIONX`, etc. - see [this `package.py` for an example](https://github.com/DUNE-DAQ/daq-release/blob/develop/spack-repos/fddaq-repo-template/packages/fdreadoutlibs/package.py).
1. Now create `spack-repos/yourenv-repo-template/packages/yourenv`, copy over `spack-repos/fddaq-repo-template/packages/fddaq/package.py` into it, and of course change the references to `fddaq` in the copied file. 
1. Add a new value to the `subset` variant in `spack-repos/coredaq-repo-template/packages/externals/package.py`, giving it the name of your environment (`variant('subset', values=('fddaq', 'yourenv' ...`)
1. Likewise for `spack-repos/coredaq-repo-template/packages/coredaq/package.py`
1. In `scripts/spack/release-setup-tools.sh`,
   edit the section which works with the `$TARGET_ABBREV` variable to
   add a map between the target abbreviation and its full name (e.g.,
   `FD` <-> `fddaq`)
1. In `scripts/checkout-daq-package.py`, edit the line which begins with `pkgs = yaml_dict.get("coredaq", []) ...` and add the name of the environment
1. Copy `scripts/templates/build-NEWENV-alma9.yml` over to `.github/workflows`, and change its name so that `NEWENV` has the environment name swapped in - e.g., `.github/workflows/build-yourenv-alma9.yml`.
1. Swap the `NEWENV` token in the body of `.github/workflows/build-yourenv-alma9.yml` as well as `NEWABBREV`, which should be replaced by the target abbreviation you chose a few steps earlier. 

At this point, you'll now have set things up to the point that you can run this workflow so as to install the environment in `/cvmfs`. 


(*) Not to be confused with the idea of a developer vs. production environment
