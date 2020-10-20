# Steps to build UPS products

1. Run the latest dunedaq docker container on a host with cvmfs installed, using bind mounts to both cvmfs root directory and a local scratch area, then:
	* setup an existing UPS products area from cvmfs, e.g. /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products
	* prepare the scratch area (bind mount from host machine): copy `.upsfiles`, `.updfiles`, `ups`, `setup*` from an exisiting UPS products area in cvmfs;
	* setup the newly prepared products area;
	* Checkout the `daq-release` package;
	* Under `daq-release/scripts/ups_build_scripts`, you will find subdirectories of package names which contains the version of the package and corresponding build script, together with its ups table file;
	* move the package subdirectories in the `ups_build_scripts` to the prepared UPS products area;
	* run the ups build script in its containing directory, e.g. `cd /scratch/products/ers/v00_26_00c; ./build_ers.sh /scratch/products e19 prof tar`;
2. After running each build script above, a tarball of the built product will be put under the prepared products area;
3. Follow the cvmfs publishing guide to publish the tarball to DUNE DAQ's cvmfs repo.
