#!/bin/bash
#
# buildJsoncpp.sh 
# (in source build required)
#

usage()
{
   echo "USAGE: `basename ${0}` <product_dir> <e19> <prof|debug> [tar]"
}

# -------------------------------------------------------------------
# shared boilerplate
# -------------------------------------------------------------------

get_this_dir() 
{
    ( cd / ; /bin/pwd -P ) >/dev/null 2>&1
    if (( $? == 0 )); then
      pwd_P_arg="-P"
    fi
    reldir=`dirname ${0}`
    thisdir=`cd ${reldir} && /bin/pwd ${pwd_P_arg}`
}

get_ssibuildshims()
{
    # make sure we can use the setup alias
    if [ -z ${UPS_DIR} ]
    then
       echo "ERROR: please setup ups"
       exit 1
    fi
    source `${UPS_DIR}/bin/ups setup ${SETUP_UPS}`
    
    setup ssibuildshims ${ssibuildshims_version} -z ${product_dir}:${PRODUCTS}
}

function setup_and_verify()
{
    # this should not complain
    echo "Finished building ${package} ${pkgver}"
    setup ${package} ${pkgver} -q ${fullqual} -z ${product_dir}:${PRODUCTS}
    echo "${package} is installed at ${HIGHFIVE_FQ_DIR}"
}


# -------------------------------------------------------------------
# start processing
# -------------------------------------------------------------------

product_dir=${1}
basequal=${2}
extraqual=${3}
maketar=${4}

if [ -z ${product_dir} ]
then
   echo "ERROR: please specify the local product directory"
   usage
   exit 1
fi

# -------------------------------------------------------------------
# package name and version
# -------------------------------------------------------------------

package=highfive
pkgver=v2_3_1
ssibuildshims_version=v1_04_13
pkgtag="v2.3.1"

make_tarball_opts=("\${product_dir}" "\${package}" "\${pkgver}" "\${fullqual}")

get_this_dir
get_ssibuildshims
source define_basics --

setup cmake v3_17_2 || ssi_die "Unable to setup cmake"

if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib64 ]
then
   eval ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball "${make_tarball_opts[@]}"
   exit 0
fi

echo "building ${package} for ${OS}-${plat}-${qualdir} (flavor ${flvr})"

# -------------------------------------------------------------------
# package dependent stuff goes here
# -------------------------------------------------------------------

mkdir -p ${pkgdir}
if [ ! -d ${pkgdir} ]
then
   echo "ERROR: failed to create ${pkgdir}"
   exit 1
fi

# declare now so we can setup
# fake ups declare
fakedb=${product_dir}/${package}/${pkgver}/fakedb
${SSIBUILDSHIMS_DIR}/bin/fake_declare_product ${product_dir} ${package} ${pkgver} ${fullqual}

setup -B ${package} ${pkgver} -q ${fullqual} -z ${fakedb}:${product_dir}:${PRODUCTS} || ssi_die "ERROR: fake setup failed"


# doing build now
cd ${pkgdir} || ssi_die "Unable to cd to ${pkgdir}"

git clone https://github.com/BlueBrain/HighFive.git || ssi_die "Unable to clone HighFive git repo into ${PWD}"
cd ${pkgdir}/HighFive
git checkout ${pkgtag}
#patch -b -p1 < ${patchdir}/patch_cmake.patch || ssi_die "Failed to apply patch"
cd ${pkgdir}

# apply patch

set -x

ncore=`${SSIBUILDSHIMS_DIR}/bin/ncores`

# NOW COMPILE and INSTALL
builddir=${pkgdir}/build
mkdir -p ${builddir}
pushd ${builddir}
echo "installing it in ${pkgdir}"
cmake  \
    -DCMAKE_INSTALL_PREFIX=${pkgdir} \
     -DHIGHFIVE_EXAMPLES=OFF \
    ${pkgdir}/HighFive || ssi_die "Failed to run CMake"
make -j $ncore || ssi_die "Failed in 'make'"
make install || ssi_die "Failed to install"
popd

# NOW clean up
#if [ ! -d ${product_dir}/${package}/${pkgver}/include ]; then
#    mv ${pkgdir}/include ${product_dir}/${package}/${pkgver}
#fi
rm -rf ${builddir}
mv ${pkgdir}/HighFive ${product_dir}/${package}/${pkgver}/source
#mv ${pkgdir}/lib64  ${pkgdir}/lib


set +x

# real ups declare
${SSIBUILDSHIMS_DIR}/bin/declare_product ${product_dir} ${package} ${pkgver} ${fullqual} || \
  ssi_die "failed to declare ${package} ${pkgver} -q ${fullqual}"

# -------------------------------------------------------------------
# common bottom stuff
# -------------------------------------------------------------------

setup_and_verify

# this must be last
if [ "${maketar}" = "tar" ]
then
   eval ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball "${make_tarball_opts[@]}"
fi

exit 0
