# Use the standalone DUNE DAQ release tarballs

## Obtaining the release tarballs

Currently the tarballs are placed on EOS and accessible via [this link](https://cernbox.cern.ch/index.php/s/iduwhr3J6J3yay5). We will shortly move these tarballs to `dune-daq`'s project. You will need to download all four tarballs in this shared EOS folder.

## Deploy the release locally

The following code snippet shows how to the set up the pre-built release locally.


```shell
WORKDIR=$PWD
mkdir tarballs
cd tarballs

## Got to https://cernbox.cern.ch/index.php/s/iduwhr3J6J3yay5
## Download all four tarballs into this directory

mkdir $WORKDIR/dunedaq-local-releases
cd $WORKDIR/dunedaq-local-releases
tar xzf $WORKDIR/tarballs/dunedaq-v2.2.0-standalone.tar.gz

cd $WORKDIR/dunedaq-local-releases/dunedaq-v2.2.0/externals
tar xj $WORKDIR/tarballs/clang-7.0.0rc3-sl7-x86_64.tar.bz2
tar xj $WORKDIR/tarballs/gcc-8.2.0-sl7-x86_64.tar.bz2
tar xj $WORKDIR/tarballs/boost-1.73.0-sl7-x86_64-e19-prof.tar.bz2
```

Now you can clean up the tarballs if you prefer.


## Use the local deployed release with a docker container

We have a `dunedaq/sl7-minimal:latest` image available for building/testing/running the `dunedaq-v2.2.0` release.

Before running the docker container, you will need to modify the release path in `$WORKDIR/dunedaq-local-releases/dunedaq-v2.2.0/dbt-settings.sh`. 

Assuming you will run the container with a bind mount `-v $WORKDIR:/scratch`, the `dune_product_dirs` should look like the following. For you convinience, the initial `dbt-settings.sh` file already contains these paths. You may need to modify it accordingly if you run the container with a different bind mount.

```shell
 dune_products_dirs=(
     "/scratch/dunedaq-local-releases/dunedaq-v2.2.0/externals"
     "/scratch/dunedaq-local-releases/dunedaq-v2.2.0/packages"
 )
```

Then run the docker container with:
```shell
docker run --rm -it -v $WORKDIR:/scratch dunedaq/sl7-minimal:latest
```

Once inside the docker container, you should be able to follow the [Compiling-and-running under v2.2.0 instructions](https://github.com/DUNE-DAQ/appfwk/wiki/Compiling-and-running-under-v2.2.0).

Note that you will need to call `dbt-create.sh` with the `-r` option to specifiy the release directory `/scratch/dunedaq-local-releases`, i.e.

```shell
dbt-create.sh -r /scratch/dunedaq-local-releases dunedaq-v2.2.0
```

Please also note that with the above docker command, only files created under `/scratch` are synced/saved to the host machine.
