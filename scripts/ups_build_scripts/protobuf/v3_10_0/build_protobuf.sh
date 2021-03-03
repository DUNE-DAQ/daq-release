#!/bin/bash
#
# buildProtobuf.sh 

# if this build script complains that it cannot find bzlib.h, 
# you need to install bzip2-devel on Scientific Linux

usage()
{
   echo "USAGE: `basename ${0}` <product_directory> <e15|e17|e19|c2|c7>  [tar]"
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
    if [ -z ${SSIBUILDSHIMS_DIR} ]
    then
       echo "ERROR: failed to setup ssibuildshims ${ssibuildshims_version}"
       exit 1
    fi
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

package=protobuf
origpkgver=v3_10_0
pkgver=${origpkgver}
ssibuildshims_version=v1_04_13
pkgdotver=`echo ${origpkgver} | sed -e 's/_/./g' | sed -e 's/^v//'`
pkgtarname=protobuf-${pkgdotver}

get_this_dir

get_ssibuildshims

source define_basics NO_TYPE

if [ "${maketar}" = "tar" ] && [ -d ${pkgdir} ]
then
   ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball ${product_dir} ${package} ${pkgver} ${fullqual}
   exit 0
fi

echo "building ${package} for ${OS}-${plat}-${qualdir} (flavor ${flvr})"

# -------------------------------------------------------------------
# package dependent stuff goes here
# -------------------------------------------------------------------

if [[ "${basequal}" == e1[045] ]]
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
  cxxflg="-std=c++17 -D_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES=1"
else
  ssi_die "Qualifier $basequal not recognized."
fi

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

allcores=`${SSIBUILDSHIMS_DIR}/bin/ncores`
(( ncore=${allcores} ))

set -x

cd ${pkgdir}
##mkdir lib
##mkdir include
mkdir src
cd src
curl -L https://github.com/protocolbuffers/protobuf/archive/v3.10.0.tar.gz |tar xz

cd protobuf-${pkgdotver}

##./autogen.sh

export CC=${cc}
export CXX=${cxx}
export FC=gfortran

./autogen.sh
./configure --prefix="${pkgdir}" --with-pic CXXFLAGS="${cxxflg}"
make -j${ncore} || ssi_die "ERROR: make failed."
#make -j${ncore} check || ssi_die "ERROR: make check failed." 
make -j${ncore} install || ssi_die "ERROR: make install failed."
#make clean || ssi_die "ERROR: make clean failed."

set +x


# real ups declare
${SSIBUILDSHIMS_DIR}/bin/declare_product ${product_dir} ${package} ${pkgver} ${fullqual} || ssi_die "failed to declare ${package} ${pkgver}"

# -------------------------------------------------------------------
# common bottom stuff
# -------------------------------------------------------------------

# this should not complain
echo "Finished building ${package} ${pkgver}"
setup ${package} ${pkgver} -q ${fullqual} -z ${product_dir}:${PRODUCTS}
echo "protobuf is installed at ${PROTOBUF_FQ_DIR}"

# fix paths for pkgconfig
pkgcnf=`ls ${PROTOBUF_FQ_DIR}/lib/pkgconfig/*.pc`
for file in ${pkgcnf}
do
  mv ${file} ${file}.bak
  cat ${file}.bak | sed -e "s%${PROTOBUF_FQ_DIR}%\${PROTOBUF_FQ_DIR}%g" > ${file}
done

# this must be last
if [ "${maketar}" = "tar" ] && [ -d ${PROTOBUF_FQ_DIR} ]
then
   ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball ${product_dir} ${package} ${pkgver} ${fullqual}
fi

exit 0
