#!/bin/bash
#

# if this build script complains that it cannot find bzlib.h, 
# you need to install bzip2-devel on Scientific Linux

usage()
{
   echo "USAGE: `basename ${0}` <product_dir> <c2|c7|e15|e17|e19> [tar]"
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
maketar=${3}

if [ -z ${product_dir} ]
then
   echo "ERROR: please specify the local product directory"
   usage
   exit 1
fi

# -------------------------------------------------------------------
# package name and version
# -------------------------------------------------------------------

package=zmq
origpkgver=v4_3_1
pkgver=v4_3_1b
ssibuildshims_version=v1_04_14
cmake_version=v3_17_2
pkgdotver=`echo ${origpkgver} | sed -e 's/_/./g' | sed -e 's/^v//'`
pkgtarfile=zeromq-${pkgdotver}.tar.gz

get_this_dir

get_ssibuildshims

source define_basics NO_TYPE

if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib ]
then
   ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball ${product_dir} ${package} ${pkgver} ${fullqual}
   exit 0
fi

echo "building ${package} for ${OS}-${plat}-${qualdir} (flavor ${flvr})"

# -------------------------------------------------------------------
# package dependent stuff goes here
# -------------------------------------------------------------------
extra_command=""
cflg="-g -O2"

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
tar xfzv ${tardir}/${pkgtarfile}

pushd zeromq-${pkgdotver}

./configure ${extra_command} --prefix=${pkgdir}

(( $? == 0 )) || ssi_die "ERROR: configure failed."
make clean || ssi_die "ERROR: make clean failed."
    
if [ $(uname) = "Darwin" ]
then
    make -j 1 || ssi_die "ERROR: make failed."
else
    make -j ${ncore} || ssi_die "ERROR: make failed."
fi

make install  || ssi_die "ERROR: make install failed."

popd

set +x

#get cppzmq headers
cppzmqdotver=4.3.0
pushd ${pkgdir}
tar xfzv ${tardir}/cppzmq-${cppzmqdotver}.tar.gz
cp cppzmq-${cppzmqdotver}/*hpp include
popd

pushd ${pkgdir}/lib
mkdir -p zmq/cmake

cat > zmq/cmake/zmq-config.cmake <<EOF
set( zmq_UPS_VERSION v${pkgdotver} )

find_library(ZMQ NAMES zmq PATHS \$ENV{ZMQ_LIB_DIR} NO_DEFAULT_PATH )
set( ZMQLIBS \${ZMQ} )
EOF

cat > zmq/cmake/zmq-config-version.cmake <<EOF
set( PACKAGE_VERSION "${pkgdotver}" )
if( "\${PACKAGE_FIND_VERSION}" VERSION_EQUAL "${pkgdotver}")
  set(PACKAGE_VERSION_EXACT 1)
endif()
if( "\${PACKAGE_FIND_VERSION_MAJOR}.\${PACKAGE_FIND_VERSION_MINOR}" EQUAL "4.1" )
  set(PACKAGE_VERSION_COMPATIBLE 4)
elseif( "\${PACKAGE_FIND_VERSION_MAJOR}" EQUAL "4" )
  # for now backward compatible if minor version is less
  if( \${PACKAGE_FIND_VERSION_MINOR}  LESS 4 )
    set(PACKAGE_VERSION_COMPATIBLE 4)
  endif()
endif()
set( ZMQ_STATIC_LIB "OFF")
EOF

popd

if [ ! -d ${pkgdir}/lib ]
then
   echo "ERROR: failed to create ${pkgdir}/lib"
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
echo "zmq is installed at ${ZMQ_BASE}"

# this must be last
if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib ]
then
   ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball ${product_dir} ${package} ${pkgver} ${fullqual}
fi

exit 0
