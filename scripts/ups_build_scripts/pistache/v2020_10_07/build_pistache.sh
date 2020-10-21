#!/bin/bash
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
    
    setup ssibuildshims ${ssibuildshims_version} -z ${product_dir}
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

package=pistache
pkgver=v2020_10_07
ssibuildshims_version=v1_04_13
cmake_version=v3_17_2

get_this_dir

get_ssibuildshims

source define_basics --

if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib64 ]
then
   ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball ${product_dir} ${package} ${pkgver} ${fullqual}
   exit 0
fi

echo "building ${package} for ${OS}-${plat}-${qualdir} (flavor ${flvr})"

# -------------------------------------------------------------------
# package dependent stuff goes here
# -------------------------------------------------------------------

if [ "${extraqual}" = "debug" ]
then
    cflg="-g -O0"
elif [ "${extraqual}" = "prof" ]
then
    cflg="-O3 -g -DNDEBUG -fno-omit-frame-pointer"
fi

if [[ "${basequal}" == e1[45] ]]
then
  cc=gcc
  cxx=g++
  cxxflg="${cflg} -std=c++14 -Wno-deprecated-declarations"
elif [[ "${basequal}" == e1[79] ]]
then
  cc=gcc
  cxx=g++
  cxxflg="${cflg} -std=c++17 -Wno-deprecated-declarations"
elif [[ "${basequal}" == c[27] ]] 
then
  cc=clang
  cxx=clang++
  cxxflg="${cflg} -std=c++17 -Wno-deprecated-declarations"
else
  echo "ERROR: unrecognized qualifier ${basequal}"
  usage
  exit 1
fi

mkdir -p ${pkgdir}
if [ ! -d ${pkgdir} ]
then
   echo "ERROR: failed to create ${pkgdir}"
   exit 1
fi
mkdir -p ${blddir}
if [ ! -d ${blddir} ]
then
   echo "ERROR: failed to create ${blddir}"
   exit 1
fi
mkdir -p ${sourcedir}
if [ ! -d ${sourcedir} ]
then
   echo "ERROR: failed to create ${sourcedir}"
   exit 1
fi

# declare now so we can setup
# fake ups declare
fakedb=${product_dir}/${package}/${pkgver}/fakedb
${SSIBUILDSHIMS_DIR}/bin/fake_declare_product ${product_dir} ${package} ${pkgver} ${fullqual}

setup -B ${package} ${pkgver} -q ${fullqual} -z ${fakedb}:${product_dir}:${PRODUCTS} || ssi_die "fake setup failed"

export CXXFLAGS="$cxxflg"
export CFLAGS="${cflg}"
export CC=${cc}
export CXX=${cxx}


ncore=`${SSIBUILDSHIMS_DIR}/bin/ncores`

set -x
cd ${sourcedir}

# Download
git clone https://github.com/oktal/pistache.git
cd ${package}
git submodule update --init

### Package specific stuff

cd ${blddir}
cmake -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DPISTACHE_BUILD_EXAMPLES=true \
        -DPISTACHE_BUILD_TESTS=true \
        -DPISTACHE_BUILD_DOCS=false \
        -DPISTACHE_USE_SSL=true \
        -DCMAKE_INSTALL_PREFIX=${pkgdir} \
        ${sourcedir}/${package}

(( $? == 0 )) || ssi_die "ERROR: cmake failed."
    
if [ $(uname) = "Darwin" ]
then
    make -j 1 || ssi_die "ERROR: make failed."
else
    make -j ${ncore} || ssi_die "ERROR: make failed."
fi

make install  || ssi_die "ERROR: make install failed."

set +x

if [ ! -d ${pkgdir}/lib64 ]
then
   echo "ERROR: failed to create ${pkgdir}/lib64"
   echo "       Something is very wrong"
   exit 1
fi

# real ups declare
${SSIBUILDSHIMS_DIR}/bin/declare_product ${product_dir} ${package} ${pkgver} ${fullqual}

# -------------------------------------------------------------------
# common bottom stuff
# -------------------------------------------------------------------

# this should not complain
echo "Finished building ${package} ${pkgver}"
setup ${package} ${pkgver} -q ${fullqual} -z ${product_dir}:${PRODUCTS}
echo "$package is installed at $(eval echo "\${${package^^*}_FQ_DIR}")"

# this must be last
if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib64 ]
then
   ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball ${product_dir} ${package} ${pkgver} ${fullqual}
fi

exit 0
