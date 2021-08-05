#!/bin/bash
#
# buildJsoncpp.sh 
# (in source build required)
#

usage()
{
   echo "USAGE: `basename ${0}` <product_dir> <e19> <prof> [tar]"
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

if [[ "${basequal}" == e1[79] ]]
then
  cc=gcc
  cxx=g++
  cxxflg="-fPIC -std=c++17"
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

package=cpr
pkgver=v1_6_2
ssibuildshims_version=v1_04_14
pkgdotver=`echo ${pkgver} | sed -e 's/_/./g' -e 's/^v//'`
pkgtar=v${pkgdotver}.tar.gz


make_tarball_opts=("\${product_dir}" "\${package}" "\${pkgver}" "\${fullqual}")

get_this_dir
get_ssibuildshims
source define_basics --


if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib64 ]
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
mkdir -p ${tardir}
wget -O ${tardir}/${pkgtar} https://github.com/whoshuu/cpr/archive/1.6.2.tar.gz
tar xf ${tardir}/${pkgtar} || ssi_die "Unable to unwind ${tardir}/${pkgtar} into ${PWD}"

# apply patch
echo ${patchdir}
cd ${pkgdir}
patch -b -p1 <${patchdir}/cpr.patch || ssi_die "Unable to apply patch"

cd ${pkgdir}/${package}-${pkgdotver} || ssi_die "Unable to cd to ${pkgdir}/${package}-${pkgdotver}"

set -x

ncore=`${SSIBUILDSHIMS_DIR}/bin/ncores`

# NOW COMPILE

setup cmake v3_17_2

CC=${cc} \
CXX=${cxx}  \
cmake ${extra_command} \
-DBUILD_CPR_TESTS=OFF \
-DUSE_SYSTEM_CURL=ON \
-DCMAKE_INSTALL_PREFIX=${pkgdir} \
${pkgdir}/${package}-${pkgdotver}
#-DCMAKE_BUILD_TYPE=Release \

make -j $ncore || ssi_die "Failed in 'make'"

# NOW INSTALL
incdir=${product_dir}/${package}/${pkgver}/include
srcdir=${product_dir}/${package}/${pkgver}/src

make install || ssi_die "Failed to install"

# NOW clean up
#if [ -d ${incdir}/${package} ]; then
#    mv ${incdir}/${package} ${incdir}
#    rm -rf ${incdir}/${package}
#fi

if [ -d ${pkgdir}/bin/${package} ]; then
    find ${pkgdir}/bin -type f -exec mv {} ${pkgdir}/bin \;
    rm -rf ${pkgdir}/bin/${package}
fi

pushd ${pkgdir}/lib64
mkdir -l cmake/cpr
cp ${pkgdir}/${package}-${pkgdotver}/cpr-config.cmake cmake/cpr/
popd


if [ ! -d ${srcdir} ]; then
    mv ${pkgdir}/${package}-${pkgdotver} ${srcdir}
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
if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib64 ]
then
   eval ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball "${make_tarball_opts[@]}"
fi

exit 0
