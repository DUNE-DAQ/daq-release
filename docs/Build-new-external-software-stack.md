# Building and deploying external packages

## Preliminary

You'll want to do the following logged onto `daq.fnal.gov` as user
`dunedaq`. Ideally you'll be logged on in a manner such that it's
unlikely your ssh connection will broken; while strides have been made
in getting the `build-ext.sh` script to be able to pick up where it
left off, it's generally better to be able to run `build-ext.sh` in
one go. Note that this takes about 2-3 hours.

## Build setup and start

In a nutshell, all you need to do is run the `build-ext.sh` script
inside a container based on the `ghcr.io/dune-daq/alma9-spack:latest`
image. To provide a bit more detail, you'll want to do the following
once you're logged into `daq.fnal.gov` as `dunedaq`:

1. Check whether there are already externals installed in `/home/nfs/dunedaq/docker-scratch/spack/externals/ext-v${EXT_VERSION}/spack-${SPACK_VERSION}` (*), and if so, that you know why they're already there.
1. Create a directory which will be the base of operations for your work, if you don't already have one
1. Inside that directory, `git clone https://github.com/DUNE-DAQ/daq-release`
1. Launch a container using the [example at the top of the `build-ext.sh` script as a guide](https://github.com/DUNE-DAQ/daq-release/blob/develop/scripts/spack/build-ext.sh). Note that `<location of local area for installation>` here would be `/home/nfs/dunedaq/docker-scratch/cvmfs_dunedaq` (as of Jul-25-2024)
1. Run `/daq-release/scripts/spack/build-ext.sh` if you want to build everything from scratch (which you do if this is your first time)
1. _or_ `/daq-release/scripts/spack/build-ext.sh false` if you want to resume an externals build (e.g. because your ssh connection got broken)

(*) Here, use `${EXT_VERSION}` and `${SPACK_VERSION}` as stand-ins for the actual externals version (e.g., `2.1`) and Spack version (e.g., `0.22.0`)

# Once the build is complete

Once complete, the externals you've built will be located in `/home/nfs/dunedaq/docker-scratch/cvmfs_dunedaq/ext-v${EXT_VERSION}/spack-${SPACK_VERSION}`. Note that you'll need to get them copied from the local area on `daq.fnal.gov` to two separate locations: (1) an externals image in which the nightly build can be performed, and (2) onto cvmfs. _Please confirm you can build a nightly in an externals image before altering cvmfs_ . In order to do so:

1. Run the [Build docker with slim externals](https://github.com/DUNE-DAQ/daq-release/actions/workflows/slim_externals.yaml) GitHub Action. _Make sure_ you add an argument to the "optional suffix for test-only externals image" field, otherwise you'll clobber the standard externals image used for the usual nightly. This Action usually takes very roughly an hour, sometimes a bit less.

1. Create a temporary branch forked off of the `develop` branch where you can modify the relevant nightly workflow YAML file (i.e., the nightly you plan to base off the externals you've built). The, modify the file so that instead of using the standard externals image, it uses the test image you created. E.g., if you provided `T` as an argument to "Build docker with slim externals", you'd stick a `T` at the end of the externals image referred to in the workflow file.

1. Run the workflow off the temporary branch, and make sure that you provide a nightly tag prefix so you don't clobber the standard nightly. Also select `yes` for whether to deploy the release to cvmfs if it builds correctly.

1. If it does, in fact, build successfully, now you can update the externals area on the actual cvmfs. Details on how to do that are [here](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-release/publish_to_cvmfs/#updating-a-particular-directory-on-cvmfs).

