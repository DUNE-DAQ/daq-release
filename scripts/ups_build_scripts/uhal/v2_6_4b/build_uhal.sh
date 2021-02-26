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

if [[ "${basequal}" == e1[0245] ]]
then
  cc=gcc
  cxx=g++
  cxxflg="-std=c++14"
elif [[ "${basequal}" == e1[79] ]]
then
  cc=gcc
  cxx=g++
  cxxflg="-std=c++17"
elif [[ "${basequal}" == c[27] ]]
then
  cc=clang
  cxx=clang++
  cxxflg="-std=c++17"
else
  ssi_die "Qualifier $basequal not recognized."
fi
extra_command="${extra_command} -DCMAKE_CXX_FLAGS=${cxxflg}"

if [ -z ${product_dir} ]
then
   echo "ERROR: please specify the local product directory"
   usage
   exit 1
fi

# -------------------------------------------------------------------
# package name and version
# -------------------------------------------------------------------

srcpkgname=ipbus-software
package=uhal
pkgver=v2_6_4b
ssibuildshims_version=v1_04_14
pkgdotver="2.6.4"
pkgtar=v${pkgdotver}.tar.gz


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


setup python v3_8_3b

# doing build now
cd ${pkgdir} || ssi_die "Unable to cd to ${pkgdir}"
mkdir -p ${tardir}
wget -O ${tardir}/${pkgtar} https://github.com/ipbus/ipbus-software/archive/v${pkgdotver}.tar.gz
tar xf ${tardir}/${pkgtar} || ssi_die "Unable to unwind ${tardir}/${pkgtar} into ${PWD}"

# apply patch
echo ${patchdir}
cd ${pkgdir}
patch -b -p1 <${patchdir}/ipbus.patch || ssi_die "Unable to apply patch"

cd ${pkgdir}/${srcpkgname}-${pkgdotver} || ssi_die "Unable to cd to ${pkgdir}/${srcpkgname}-${pkgdotver}"

set -x

ncore=`${SSIBUILDSHIMS_DIR}/bin/ncores`

# NOW COMPILE
export Set=uhal
export EXTERN_BOOST_INCLUDE_PREFIX=$BOOST_INC
export EXTERN_BOOST_LIB_PREFIX=$BOOST_LIB
export EXTERN_PUGIXML_INCLUDE_PREFIX=$PUGIXML_INC
export EXTERN_PUGIXML_LIB_PREFIX=$PUGIXML_LIB
Set=uhal \
CC=${cc} \
CXX=${cxx}  \
make -j 8 || ssi_die "Failed in 'make'"
#make -j $ncore || ssi_die "Failed in 'make'"

# NOW INSTALL
incdir=${product_dir}/${package}/${pkgver}/include
srcdir=${product_dir}/${package}/${pkgver}/src
Set=uhal \
make install prefix=${pkgdir} includedir=${incdir} || ssi_die "Failed to install"

# NOW clean up
#if [ -d ${incdir}/${package} ]; then
#    mv ${incdir}/${package} ${incdir}
#    rm -rf ${incdir}/${package}
#fi

if [ -d ${pkgdir}/bin/${package} ]; then
    find ${pkgdir}/bin -type f -exec mv {} ${pkgdir}/bin \;
    rm -rf ${pkgdir}/bin/${package}
fi

pushd ${pkgdir}/lib
for i in `find . -type l`; do j=`echo $i|grep -o ".*.so" `; rm -f $i; ln -s $j.2.6.4 $i; done
popd


if [ ! -d ${srcdir} ]; then
    mv ${pkgdir}/${srcpkgname}-${pkgdotver} ${srcdir}
fi

set +x

# real ups declare
## 
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
