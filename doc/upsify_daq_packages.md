# How to make UPS products of currently built/installed DAQ packages

A python3 script has been prepared for this purpose. It is [upsify-daq-pkgs.py](https://github.com/DUNE-DAQ/daq-release/blob/master/scripts/upsify-daq-pkgs.py).

```
usage: upsify_daq_pkgs.py [-h] -w WORK_DIR -t TARBALL_DIR [-p PACKAGE_NAME] [-e EQUALIFIER] [-d]

Make UPS products for DAQ packages installed under working directory.

optional arguments:
  -h, --help            show this help message and exit
  -w WORK_DIR, --work-dir WORK_DIR
                        Path to DAQ software working directory; (default: None)
  -t TARBALL_DIR, --tarball-dir TARBALL_DIR
                        Path to the destination tarball directory; (default: None)
  -p PACKAGE_NAME, --package-name PACKAGE_NAME
                        Name of package to be 'UPSified'; if not supplied, all currently installed packages will be 'UPSified'; (default: None)
  -e EQUALIFIER, --equalifier EQUALIFIER
                        e qualifier; (default: e19)
  -d, --debug           flag for the 'debug' qualifer ('prof' if unset). (default: False)

Questions and comments to dingpf@fnal.gov


```

Just as shown in the helper above, assuming you have a working directory (`$WORK_DIR`)set up by following the [Compiling and Running](https://github.com/DUNE-DAQ/appfwk/wiki/Compiling-and-running) wiki page, you can run the script as the following:

```
./upsify-daq-pkgs.py -w $WORK_DIR -t $TABALL_DIR [-e e19] [-d] [-p pkg_name]
```

where `$TARBALL_DIR` is a directory where you want the generated tarballs saved.


A few things to note as of now (Nov 24, 2020):
1. The script supports making UPS products of any e-qualifers and `debug` or `prof` build;
2. It makes SL7-only UPS products;
3. DAQ packages by default are built without `debug` options, and only on SL7 or CentOS 7.
