#!/bin/bash
#

# build_pyyaml.sh 

# The build is qualified because it depends upon LAPACK, which is qualified.
usage()
{
   echo "USAGE: `basename ${0}` <product_directory> <p383b> [tar]"
}

# -------------------------------------------------------------------
# shared boilerplate
# -------------------------------------------------------------------

get_this_dir() 
{
    reldir=`dirname ${0}`
    thisdir=`cd ${reldir} && pwd -P`
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
extraqual=${2}
maketar=${3}

if [[ "${maketar:-tar}" != "tar" ]]; then
  echo "WARNING: build type specifier ${maketar} no longer accepted: ignoring." 1>&2
  maketar=${4}
fi

if [ -z ${product_dir} ]
then
   echo "ERROR: please specify the local product directory"
   usage
   exit 1
fi

# -------------------------------------------------------------------
# package name and version
# -------------------------------------------------------------------

package=pyyaml
origpkgver=v5_3_1
pkgver=${origpkgver}
ssibuildshims_version=v1_04_13
pkgdotver=`echo ${origpkgver} | sed -e 's/_/./g' | sed -e 's/^v//'`
pkgtarname=${package}-${pkgdotver}

get_this_dir
get_ssibuildshims

source ssi_functions --

source define_basics NO_BASE_QUAL

if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib ]
then
   ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball ${product_dir} ${package} ${pkgver} ${fullqual}
   exit 0
fi

echo "building ${package} for ${OS}-${plat}-${qualdir} (flavor ${flvr})"

# -------------------------------------------------------------------
# package dependent stuff goes here
# -------------------------------------------------------------------

# declare now so we can setup
# fake ups declare
fakedb=${product_dir}/${package}/${pkgver}/fakedb
${SSIBUILDSHIMS_DIR}/bin/fake_declare_product ${product_dir} ${package} ${pkgver} ${fullqual}

setup -B ${package} ${pkgver} -q ${fullqual} -z ${fakedb}:${product_dir}:${PRODUCTS} || ssi_die "fake setup failed"

if [ ${OS1} = "Darwin" ]
then
   PATH=`dropit /usr/bin/get-cert $PATH`
fi

set -x

# work in a build directory, which will not be part of any distribution
python_shortdotversion=${PYTHON_VERSION#v}
python_shortdotversion=${python_shortdotversion//_/.}
python_shortdotversion=${python_shortdotversion%.*}

mkdir -p ${pkgdir}/lib || ssi_die "failed to create ${pkgdir}/lib"
mkdir -p ${blddir} || ssi_die "failed to create ${blddir}"
rm -rf ${blddir}/*

cd ${blddir} || ssi_die "Unable to cd into ${blddir}"
wget https://github.com/yaml/pyyaml/archive/${pkgdotver}.tar.gz
tar xf ${pkgdotver}.tar.gz || ssi_die "Failed to untar ${pkgdotver}.tar.gz"
cd ${package}-${pkgdotver} || ssi_die "Failed to cd into ${package}-${pkgdotver}"

python setup.py --without-libyaml build || ssi_die "python setup.py build failed"
mv build/lib.linux* build/lib
python -m compileall ./build/lib

mv build/lib ${pkgdir}

# Maybe run some tests.
set +x

#replace hardwired python path 
#sed -i'' -e '1s/^.*$/#!\/usr\/bin\/env python /' ${pkgdir}/bin/f2py || ssi_die "Failed to fix shebang for f2py"

# real ups declare
${SSIBUILDSHIMS_DIR}/bin/declare_product ${product_dir} ${package} ${pkgver} ${fullqual} || \
  ssi_die "failed to declare ${package} ${pkgver} -q ${fullqual}"

# -------------------------------------------------------------------
# common bottom stuff
# -------------------------------------------------------------------

# this should not complain
echo "Finished building ${package} ${pkgver}"
setup -B ${package} ${pkgver} -q ${fullqual} -z ${product_dir}:${PRODUCTS}
echo "$package is installed at $(eval echo "\${${package^^*}_FQ_DIR}")"

# this must be last
if [ "${maketar}" = "tar" ] && [ -d ${pkgdir}/lib ]
then
   ${SSIBUILDSHIMS_DIR}/bin/make_distribution_tarball ${product_dir} ${package} ${pkgver} ${fullqual}
fi

exit 0
