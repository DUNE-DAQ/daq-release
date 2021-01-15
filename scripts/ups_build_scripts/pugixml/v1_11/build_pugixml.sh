#!/bin/bash
#
# buildJsoncpp.sh 
# (in source build required)
#

usage()
{
   echo "USAGE: `basename ${0}` <product_dir> <e15|e17|e19> [tar]"
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
    echo "${package} is installed at ${PUGIXML_FQ_DIR}"
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

package=pugixml
pkgver=v1_11
ssibuildshims_version=v1_04_14
pkgdotver=`echo ${pkgver} | sed -e 's/_/./g' -e 's/^v//'`
pkgtar=pugixml-${pkgdotver}.tar.gz

make_tarball_opts=("-e src:include \${product_dir}" "\${package}" "\${pkgver}" "\${fullqual}")

get_this_dir
get_ssibuildshims
source define_basics NO_TYPE --

if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib ]
then
   eval ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball "${make_tarball_opts[@]}"
   exit 0
fi

echo "building ${package} for ${OS}-${plat}-${qualdir} (flavor ${flvr})"

# -------------------------------------------------------------------
# package dependent stuff goes here
# -------------------------------------------------------------------

if [[ "${basequal}" == e15 ]]
then
  cxxflg="-std=c++14"
elif [[ "${basequal}" == e1[79] ]]
then
  cxxflg="-std=c++17"
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


cd ${pkgdir} || ssi_die "Unable to cd to ${pkgdir}"

mkdir -p ${tardir}
wget -O ${tardir}/${pkgtar} https://github.com/zeux/pugixml/archive/v${pkgdotver}.tar.gz
echo " https://github.com/zeux/pugixml/archive/v${pkgdotver}.tar.gz"
tar zxvf ${tardir}/${pkgtar} || ssi_die "Unable to unwind ${tardir}/${pkgtar} into ${PWD}"

cd ${pkgdir}/pugixml-${pkgdotver} || ssi_die "Unable to cd to ${pkgdir}/pugixml-${pkgdotver}"

# We need cmake
setup cmake v3_17_2


set -x


srcdir=${product_dir}/${package}/${pkgver}/src
incdir=${product_dir}/${package}/${pkgver}/include
libdir=${pkgdir}/lib

mkdir -p ${libdir}
mkdir -p ${incdir}


cmake ${extra_command} \
	-DCMAKE_INSTALL_LIBDIR=${libdir} \
	-DCMAKE_INSTALL_INCLUDEDIR=${incdir} \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_STATIC_LIBS=OFF \
	${pkgdir}/pugixml-${pkgdotver} || ssi_die "Failed to run CMake"
make ; make install


set +x
# clean up .o files in libdir, and move binaries to bindir

if [ ! -d ${srcdir} ]
then
    mv ${pkgdir}/pugixml-${pkgdotver} ${srcdir}
else
    rm -rf ${pkgdir}/pugixml-${pkgdotver} 
fi


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
