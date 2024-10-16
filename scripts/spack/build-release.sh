#!/bin/bash

if (( $# < 4 || $# > 5 )); then
    echo "Usage: $( basename $0 ) <desired base release directory> 
                        <desired detector release directory> 
                        <build type (fd, nd, or core)> 
                        <OS (almalinux9 or scientific7)>
                        (optional default repo branch (nightly only, default is develop) )" >&2
    exit 1
fi

export BASE_RELEASE_DIR=$1
export DET_RELEASE_DIR=$2
export DET=$3
export OS=$4

export DEFAULT_BRANCH="develop"
if [[ -n $5 ]]; then
    export DEFAULT_BRANCH=$5
fi

if [[ $DET != "core" && $DET != "fd" && $DET != "nd" ]]; then
    echo "Type of build needs to be specified as \"core\" (common packages only), \"fd\" (far detector stack) or \"nd\" (near detector stack); exiting..." >&2
    exit 2
fi

if [[ $OS != "almalinux9" && $OS != "scientific7" ]]; then
    echo "OS needs to be specified either as \"almalinux9\" or \"scientific7\"; exiting..." >&2
    exit 3
fi

export DAQ_RELEASE_REPO=$PWD/$(dirname "$0")/../..
. $DAQ_RELEASE_REPO/.github/workflows/wf-setup-tools.sh || exit 3

if [[ $DET == "core" ]]; then
    export SPACK_AREA=$BASE_SPACK_AREA
elif [[ $DET == "nd" || $DET == "fd" ]]; then
    export SPACK_AREA=$DET_SPACK_AREA
fi

mkdir -p $SPACK_AREA
cd $SPACK_AREA
get_spack

if [[ "$DET" == "core" ]]; then
  daqify_spack_environment base
  release_yaml=$( get_release_yaml "base" )
  base_release_arg=""
  export RELEASE_TAG=$BASE_RELEASE_TAG
elif [[ "$DET" == "fd" || "$DET" == "nd" ]]; then
  daqify_spack_environment det
  release_yaml=$( get_release_yaml "$DET" )
  base_release_arg="--base-release ${BASE_RELEASE_TAG}"
  export RELEASE_TAG=$DET_RELEASE_TAG
fi

if [[ $RELEASE_TYPE == "nightly" || $RELEASE_TYPE == "production_v4" ]]; then
  branch_arg="-b "${DEFAULT_BRANCH}
else
  branch_arg=""
fi

cd $DAQ_RELEASE_REPO

spack_template_dir=spack-repos/${DET}daq-repo-template

echo python3 scripts/spack/make-release-repo.py -u \
  -i ${release_yaml} \
  -t $spack_template_dir \
  -r ${RELEASE_TAG} \
  -o ${SPACK_AREA}/spack-installation \
  ${base_release_arg} \
  ${branch_arg} \
  || exit 5


python3 scripts/spack/make-release-repo.py -u \
  -i ${release_yaml} \
  -t $spack_template_dir \
  -r ${RELEASE_TAG} \
  -o ${SPACK_AREA}/spack-installation \
  ${base_release_arg} \
  ${branch_arg} \
  || exit 5


cd $SPACK_AREA

spack clean -m 
spack spec -l --reuse ${DET}daq@${RELEASE_TAG}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 > $SPACK_AREA/spec_${DET}daq_log.txt 2>&1
retval=$?

cat $SPACK_AREA/spec_${DET}daq_log.txt 

if [[ $retval != 0 ]]; then
    exit 20
fi

build_dbe=false
if [[ $DET == "core" ]]; then
    spack spec -l --reuse dbe%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 > $SPACK_AREA/spec_dbe_log.txt 2>&1
    retval=$?    

    cat $SPACK_AREA/spec_dbe_log.txt

    if [[ $retval == 0 ]]; then
	build_dbe=true
    else
	build_dbe=false
        echo "Building dbe does not appear to be possible. As this is not (necessarily) an error, will continue..."
    fi
fi

spack install --reuse ${DET}daq@${RELEASE_TAG}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 || exit 7

if $build_dbe; then
    spack install --reuse dbe%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 || exit 8
fi

if [[ "$DET" == "fd" || "$DET" == "nd" ]]; then
    # Generate pyvenv_requirements.txt
    spack load ${DET}daq@${RELEASE_TAG} || exit 9

    cd $DAQ_RELEASE_REPO
    echo /usr/bin/python3 scripts/spack/make-release-repo.py \
        -o ${SPACK_AREA} \
        --pyvenv-requirements \
        -i ${release_yaml}

    /usr/bin/python3 scripts/spack/make-release-repo.py \
        -o ${SPACK_AREA} \
        --pyvenv-requirements \
        -i ${release_yaml} \
        || exit 10

    python -m venv --prompt dbt ${SPACK_AREA}/.venv
    source ${SPACK_AREA}/.venv/bin/activate

    python -m pip install -r ${SPACK_AREA}/pyvenv_requirements.txt || exit 11

    pushd $DET_RELEASE_DIR
    cp $DAQ_RELEASE_REPO/$( dirname $release_yaml )/dbt-build-order.cmake .
    tar zcf venv.tar.gz .venv/
    popd
    
fi

echo "Files in $SPACK_AREA :"
ls -ltr $SPACK_AREA

spack clean -a
