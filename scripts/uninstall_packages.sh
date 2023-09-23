
# Start with externals

. /cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-v1.1/spack-0.20.0-gcc-12.1.0/spack-installation/share/spack/setup-env.sh

for package in cetmodules ca-certificates-mozilla texinfo meson autoconf-archive autoconf \
	       automake libtool czmq docbook-xsl docbook-xml cppzmq yaml-cpp gawk \
	       bison diffutils re2c \
	       bdftopcf mkfontdir gperf which flex findutils nasm xkbcomp \
	       cmake ninja gdb \
	       py-pip py-wheel py-hatch-fancy-pypi-readme py-flit-core py-hatch-vcs py-calver py-cython; do
    spack uninstall -y --all $package
done


for package in catch2; do
    spack uninstall -y --all --force $package
done
