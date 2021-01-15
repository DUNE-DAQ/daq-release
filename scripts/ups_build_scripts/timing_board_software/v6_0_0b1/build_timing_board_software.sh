#!/bin/bash
#
# buildJsoncpp.sh 
# (in source build required)
#

usage()
{
   echo "USAGE: `basename ${0}` <product_dir> <e15|e17> <prof> [tar]"
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
    echo "${package} is installed at ${UHAL_FQ_DIR}"
}


# -------------------------------------------------------------------
# start processing
# -------------------------------------------------------------------

product_dir=${1}
basequal=${2}
extraqual=${3}
maketar=${4}

orig_basequal=$basequal

if [ -z ${product_dir} ]
then
   echo "ERROR: please specify the local product directory"
   usage
   exit 1
fi

# -------------------------------------------------------------------
# package name and version
# -------------------------------------------------------------------

srcpackage=timing-board-software-relval-v6.0.0-b1
package=timing_board_software
pkgver=v6_0_0b1
ssibuildshims_version=v1_04_14
pkgdotver=`echo ${pkgver} | sed -e 's/_/./g' -e 's/^v//'`
pkgtar=${srcpackage}.tar.gz



make_tarball_opts=("\${product_dir}" "\${package}" "\${pkgver}" "\${fullqual}")

get_this_dir
get_ssibuildshims
source define_basics --


if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib ]
then
   eval ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball "${make_tarball_opts[@]}"
   exit 0
fi

echo "building ${package} for ${OS}-${plat}-${qualdir} (flavor ${flvr})"

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
# Downloading this from gitlab@cern requries authentication. Skipping it now.
#mkdir -p ${tardir}
#wget -O ${tardir}/${pkgtar} https://gitlab.cern.ch/protoDUNE-SP-DAQ/timing-board-software/-/archive/relval/v6.0.0/b1/timing-board-software-relval-v6.0.0-b1.tar.gz
tar xf ${tardir}/${pkgtar} || ssi_die "Unable to unwind ${tardir}/${pkgtar} into ${PWD}"

# apply modifications

pushd ${pkgdir}/${srcpackage} || ssi_die "Unable to cd to ${pkgdir}/${srcpkgname}-${pkgdotver}"

if [[ "${orig_basequal}" == e1[0245] ]]
then
  sed -i 's/c++11/c++14/g' config/*
elif [[ "${orig_basequal}" == e1[79] ]]
then
  sed -i 's/c++11/c++17/g' config/*
else
  ssi_die "Qualifier $orig_basequal not recognized."
fi

set -x

ncore=`${SSIBUILDSHIMS_DIR}/bin/ncores`

# NOW COMPILE
cd core
make build -j $ncore || ssi_die "Failed in 'make'"


# NOW INSTALL
incdir=${product_dir}/${package}/${pkgver}/include
libdir=${pkgdir}/lib
srcdir=${product_dir}/${package}/${pkgver}/src

# NOW clean up
if [ -d include ]; then
    mkdir -p ${incdir}
    cp -pr include/* ${incdir}
fi
if [ -d lib ]; then
    mkdir -p ${libdir}
    cp -pr lib/* ${libdir}
fi

if [ ! -d ${srcdir} ]; then
    mv ${pkgdir}/${srcpackage} ${srcdir}
fi




set +x

# real ups declare
${SSIBUILDSHIMS_DIR}/bin/declare_product ${product_dir} ${package} ${pkgver} ${fullqual} || \
  ssi_die "failed to declare ${package} ${pkgver} -q ${fullqual}"

# -------------------------------------------------------------------
# common bottom stuff
# -------------------------------------------------------------------

setup_and_verify

# this must be last
if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib ]
then
   eval ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball "${make_tarball_opts[@]}"
fi

exit 0
