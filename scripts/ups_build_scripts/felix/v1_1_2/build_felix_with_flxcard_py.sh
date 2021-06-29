#!/bin/bash
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
    echo "${package} is installed at ${NLOHMANN_JSON_FQ_DIR}"
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

package=felix
pkgver=v1_1_2
ssibuildshims_version=v1_04_13

make_tarball_opts=("\${product_dir}" "\${package}" "\${pkgver}" "\${fullqual}")

get_this_dir
get_ssibuildshims
source define_basics --

# declare now so we can setup
# fake ups declare
fakedb=${product_dir}/${package}/${pkgver}/fakedb
${SSIBUILDSHIMS_DIR}/bin/fake_declare_product ${product_dir} ${package} ${pkgver} ${fullqual}

setup -B ${package} ${pkgver} -q ${fullqual} -z ${fakedb}:${product_dir}:${PRODUCTS} || ssi_die "ERROR: fake setup failed"

setup cmake v3_17_2 || ssi_die "Unable to setup cmake"
#setup python v3_8_3b || ssi_die "Unable to setup python"
setup nlohmann_json v3_9_0c -q e19:prof || ssi_die "Unable to setup nlohmann_json"
setup boost v1_73_0 -q e19:prof || ssi_die "Unable to setup boost"
setup yaml_cpp v0_6_3 -q e19:prof || ssi_die "Unable to setup yaml_cpp"
#setup pybind11 v2_6_2 -q e19:prof || ssi_die "Unable to setup pybind11"

export CMAKE_COMPILER=gcc8
export BOOST_VERSION=1_73
export PYTHON_VERSION=3.8.3
export BINARY_TAG="x86_64-centos7-gcc8-opt"
export CMAKE_PREFIX_PATH="${CMAKE_PROJECT_PATH} $(dirname $(dirname $(readlink -f ${BASH_SOURCE[0]})))/cmake":$CMAKE_PREFIX_PATH
export FC=`which gfortran`
export CC=`which gcc`
export CXX=`which g++`
export COMPILER_PATH=${GCC_FQ_DIR}

echo "========== CMAKE_PREFIX_PATH"
echo $CMAKE_PREFIX_PATH


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


# doing build now
cd ${pkgdir} || ssi_die "Unable to cd to ${pkgdir}"

# checkout list of 

git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/software.git || ssi_die "Unable to clone software git repo into ${PWD}"
cd software
git checkout bb41bf8

git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/cmake_tdaq.git || ssi_die "Unable to clone cmake_tdaq git repo into ${PWD}"
pushd cmake_tdaq && git checkout 3f176ac && popd
sed -i '2 i set(NOLCG TRUE)' cmake_tdaq/cmake/modules/FELIX.cmake
export PATH=${pkgdir}/software/cmake_tdaq/bin:$PATH

git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/drivers_rcc.git || ssi_die "Unable to clone drivers_rcc git repo into ${PWD}"
pushd drivers_rcc && git checkout 7513ef2 && popd
git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/flxcard.git || ssi_die "Unable to clone flxcard git repo into ${PWD}"
pushd flxcard && git checkout 8208c3a && popd
git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/regmap.git || ssi_die "Unable to clone regmap git repo into ${PWD}"
pushd regmap && git checkout 48a6680 && popd
git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/packetformat.git || ssi_die "Unable to clone packetformat git repo into ${PWD}"
pushd packetformat && git checkout a84931e && popd
git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/client-template.git || ssi_die "Unable to clone client-template git repo into ${PWD}"
pushd client-template && git checkout 390ec87 && popd
git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/ftools.git || ssi_die "Unable to clone ftools git repo into ${PWD}"
pushd ftools && git checkout 1398314 && popd
git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/flxcard_py.git || ssi_die "Unable to clone flxcard_py git repo into ${PWD}"
pushd flxcard_py && git checkout 61001bd6 && popd
git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/external-catch.git external/catch || ssi_die "Unable to clone external-catch git repo into ${PWD}"
pushd external/catch && git checkout 6a9aa08 && popd
git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-felix/external-pybind11.git external/pybind11 || ssi_die "Unable to clone external-pybind11 git repo into ${PWD}"
pushd external/pybind11 && git checkout b75d654 && popd



cp ${product_dir}/${package}/${pkgver}/ftools_CMakeLists.txt ftools/CMakeLists.txt
cp ${product_dir}/${package}/${pkgver}/flxcard_py_CMakeLists.txt flxcard_py/CMakeLists.txt
cmake_config x86_64-centos7-gcc8-opt
pushd x86_64-centos7-gcc8-opt

set -x

ncore=`${SSIBUILDSHIMS_DIR}/bin/ncores`
make -j $ncore || ssi_die "Failed in 'make'"
popd

exit 10

mkdir -p ${pkgdir}/include
mkdir -p ${pkgdir}/lib
mkdir -p ${pkgdir}/bin


cp -r drivers_rcc ${pkgdir}/
cp -r regmap/regmap ${pkgdir}/include/
cp -r drivers_rcc/cmem_rcc ${pkgdir}/include/
cp -r drivers_rcc/rcc_error ${pkgdir}/include/
cp -r flxcard/flxcard ${pkgdir}/include/
cp -r flxcard_py/flxcard_py ${pkgdir}/include/
cp -r packetformat/packetformat ${pkgdir}/include/

cp drivers_rcc/lib64/lib* ${pkgdir}/lib
cp x86_64-centos7-gcc8-opt/flxcard/lib* ${pkgdir}/lib
cp x86_64-centos7-gcc8-opt/packetformat/lib* ${pkgdir}/lib
cp x86_64-centos7-gcc8-opt/regmap/lib* ${pkgdir}/lib
cp x86_64-centos7-gcc8-opt/drivers_rcc/lib* ${pkgdir}/lib
cp x86_64-centos7-gcc8-opt/ftools/libFlxTools* ${pkgdir}/lib
cp -r ${product_dir}/${package}/${pkgver}/cmake ${pkgdir}/lib

cp x86_64-centos7-gcc8-opt/flxcard/flx-* ${pkgdir}/bin
cp x86_64-centos7-gcc8-opt/ftools/f* ${pkgdir}/bin


cd ${pkgdir}
rm -rf ${pkgdir}/software

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
