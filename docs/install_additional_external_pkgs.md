# Add additional external packages into spack

By default, the spack instances used in a development environment are the ones deployed on cvmfs, which is read-only. This makes it impossible for developers to test new versions of external packages by themselves. The new packages or new version of packages need to be built, installed and deployed to cvmfs first before developers can use them. To speed up the testing cycle (and save SW coordinators work), it is best to provide a way for developers to install new packages locally, and make the local installation usable in the DAQ development environment. This document shows the procedure to achieve this.


## Install a local spack instance

Here are the steps/commands to install a local spack instance. Set `SPACK_DIR` to an empty directory where you want to install spack, and replace `last_successful` to the release you want to work with.

```bash

export SPACK_DIR=$HOME/spack-installation

# setup a DUNE DAQ release
# You can also replace the three lines below with "source env.sh" in an existing workarea.

source /cvmfs/dunedaq.opensciencegrid.org/setup_dunedaq.sh
setup_dbt latest
dbt-setup-release -n last_successful


mkdir -p $SPACK_DIR
export SPACK_VERSION=$(spack --version)

cd $SPACK_DIR
wget https://github.com/spack/spack/archive/refs/tags/v${SPACK_VERSION}.tar.gz
tar xf v${SPACK_VERSION}.tar.gz
rm -f v${SPACK_VERSION}.tar.gz

mkdir -p spack-${SPACK_VERSION}/local-repo/packages
echo "repo:" > spack-${SPACK_VERSION}/local-repo/repo.yaml
echo "  namespace: 'local-repo'" >> spack-${SPACK_VERSION}/local-repo/repo.yaml
sed -i '2 i  \  - '"$SPACK_DIR"'/spack-'"$SPACK_VERSION"'/local-repo' spack-${SPACK_VERSION}/etc/spack/defaults/repos.yaml

# Copy over configs from the latest nightly release.
cp $SPACK_ROOT/etc/spack/defaults/linux/compilers.yaml spack-${SPACK_VERSION}/etc/spack/defaults/linux
cp $SPACK_ROOT/etc/spack/defaults/config.yaml spack-${SPACK_VERSION}/etc/spack/defaults

cp $SPACK_ROOT/etc/spack/defaults/repos.yaml spack-${SPACK_VERSION}/etc/spack/defaults
cp $SPACK_ROOT/etc/spack/defaults/upstreams.yaml spack-${SPACK_VERSION}/etc/spack/defaults

sed -i '2 i  \  '"$SPACK_RELEASE"':' spack-${SPACK_VERSION}/etc/spack/defaults/upstreams.yaml
sed -i '3 i  \    install_tree: '"$SPACK_ROOT"'/opt/spack' spack-${SPACK_VERSION}/etc/spack/defaults/upstreams.yaml

```

Several things have been done in the steps above:

1. set up a DUNE DAQ base release;
2. installed a new spack instance using the same version of spack as the base release;
3. created an empty local spack repository, and added it to the local spack instance. This is useful when developers need to put new recipe files for some external packages;
4. copied over the configuration and compiler settings from the base release;
5. stack the local spack instance over the base release, i.e. use the spack instance of the base release as an upstream instance to the local spack instance.


## Use the local instance

Using the local instance is as simple as:
1. go to the workarea based on the same base release as used by the local spack instance, and do `source env.sh`;
2. source the setup file in the local spack instance.


```bash
cd <your-work-area>
source env.sh


export SPACK_DIR=$HOME/spack-installation
source $SPACK_DIR/spack-$(spack --version)/share/spack/setup-env.sh
```

## Add additional packages to the local instance 

You can modify an existing external package's recipe file by copying it to the local spack repo, and then reinstall the package. Since spack may find the same version already exists on cvmfs, you may need to advance the version number in the recipe file to "fool" spack so that it can do a new installation.

For example:

```bash
cp -pr /cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-v1.0/spack-0.18.1-gcc-12.1.0/spack-0.18.1/spack-repo-externals/packages/trace $SPACK_ROOT/local-repo/packages

spack edit trace

# $SPACK_ROOT/local-repo/packages/trace/packge.py is now opened in an editor
# You can add a new version by adding a new line of "version('<new_version_number>', commit='<commit_hash>')

spack install --reuse trace@<new_version_number>%gcc@12.1.0 arch=linux-scientific7-broadwell

```

