#!/bin/bash

if (( $# != 4 )); then
    echo "Usage: $( basename $0 ) <desired base release directory> 
                        <desired detector release directory> 
                        <build type (fd, nd, or dune)> 
                        <OS (almalinux9 or scientific7)>" >&2
    exit 1
fi

export BASE_RELEASE_DIR=$1
export DET_RELEASE_DIR=$2
export DET=$3
export OS=$4

if [[ $DET != "dune" && $DET != "fd" && $DET != "nd" ]]; then
    echo "Type of build needs to be specified as \"dune\" (common packages only), \"fd\" (far detector stack) or \"nd\" (near detector stack); exiting..." >&2
    exit 2
fi

if [[ $OS != "almalinux9" && $OS != "scientific7" ]]; then
    echo "OS needs to be specified either as \"almalinux9\" or \"scientific7\"; exiting..." >&2
    exit 3
fi

export DAQ_RELEASE_REPO=$PWD/$(dirname "$0")/../..
. $DAQ_RELEASE_REPO/.github/workflows/wf-setup-tools.sh || exit 3

if [[ $DET == "dune" ]]; then
    export SPACK_AREA=$BASE_SPACK_AREA
elif [[ $DET == "nd" || $DET == "fd" ]]; then
    export SPACK_AREA=$DET_SPACK_AREA
fi

mkdir -p $SPACK_AREA
cd $SPACK_AREA
get_spack

if [[ "$DET" == "dune" ]]; then
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

if [[ $RELEASE_TYPE == "nightly" ]]; then
  branch_arg="-b "${INPUT_BRANCH:-"develop"}
else
  branch_arg=""
fi

cd $DAQ_RELEASE_REPO

python3 scripts/spack/make-release-repo.py -u \
  -i ${release_yaml} \
  -t spack-repos/${DET}daq-repo-template \
  -r ${RELEASE_TAG} \
  -o ${SPACK_AREA}/spack-${SPACK_VERSION} \
  ${base_release_arg} \
  ${branch_arg}


cd $SPACK_AREA

spack spec --reuse ${DET}daq@${RELEASE_TAG}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 |  tee $SPACK_AREA/spec_${DET}daq_log.txt
spack install --reuse ${DET}daq@${RELEASE_TAG}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64

if [[ "$DET" == "fd" || "$DET" == "nd" ]]; then
    # Generate pyvenv_requirements.txt
    spack load ${DET}daq@${RELEASE_TAG}

    cd $DAQ_RELEASE_REPO
    /usr/bin/python3 scripts/spack/make-release-repo.py \
        -o ${SPACK_AREA}/../ \
        --pyvenv-requirements \
        -i ${release_yaml}

    python -m venv --prompt dbt ${SPACK_AREA}/../.venv
    source ${SPACK_AREA}/../.venv/bin/activate

    python -m pip install -r ${SPACK_AREA}/../pyvenv_requirements.txt

    pushd ${DET_RELEASE_DIR}
    cp $( basedir $release_yaml )/dbt-build-order.cmake .
    tar zcf venv.tar.gz .venv/
    popd

fi

spack clean -a
