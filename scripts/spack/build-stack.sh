#!/bin/bash

if (( $# != 3 )); then
    echo "Usage: $( basename $0 ) <nightly tag (YY-MM-DD)> <build type (fd, nd, or dune)> <OS (almalinux9 or scientific7)>" >&2
    exit 1
fi

export NIGHTLY_TAG=$1
export DET=$2
export OS=$3

if ! [[ "$NIGHTLY_TAG" =~ [0-9][0-9]-[0-9][0-9]-[0-9][0-9] ]]; then
    echo "Nightly tag needs to be of the format YY-MM-DD; exiting..." >&2
    exit 1
fi

if [[ $DET != "dune" && $DET != "fd" && $DET != "nd" ]]; then
    echo "Type of build needs to be specified as \"dune\" (common packages only), \"fd\" (far detector stack) or \"nd\" (near detector stack); exiting..." >&2
    exit 2
fi

if [[ $OS != "almalinux9" && $OS != "scientific7" ]]; then
    echo "OS needs to be specified either as \"almalinux9\" or \"scientific7\"; exiting..." >&2
    exit 3
fi

if [[ $OS == "almalinux9" ]]; then
    export TAG_PREFIX="NA"
elif [[ $OS == "scientific7" ]]; then
    export TAG_PREFIX="N"
fi

if [[ $DET == "dune" ]]; then
    export RELEASE_TAG=${TAG_PREFIX}B${NIGHTLY_TAG}
elif [[ $DET == "fd" ]]; then
    export RELEASE_TAG=${TAG_PREFIX}FD${NIGHTLY_TAG}
elif [[ $DET == "nd" ]]; then
    export RELEASE_TAG=${TAG_PREFIX}ND${NIGHTLY_TAG}
fi

export DAQ_RELEASE_REPO=$PWD/$(dirname "$0")/../..
. $DAQ_RELEASE_REPO/.github/workflows/wf-setup-tools.sh || exit 3

mkdir -p $SPACK_AREA
cd $SPACK_AREA
get_spack

export FEATURE_BRANCH=${INPUT_BRANCH:-"develop"}

if [[ "$DET" == "dune" ]]; then
  daqify_spack_environment base
  base_release_arg=""
elif [[ "$DET" == "fd" || "$DET" == "nd" ]]; then
  daqify_spack_environment det
  base_release_arg="--base-release ${BASE_RELEASE_TAG}"
else
  echo "There's an error in this script's logic"
  exit 1
fi

cd $DAQ_RELEASE_REPO
python3 scripts/spack/make-release-repo.py -u \
  -b ${FEATURE_BRANCH} \
  -i configs/${DET}daq/${DET}daq-develop/release.yaml \
  -t spack-repos/${DET}daq-repo-template \
  -r ${RELEASE_TAG} \
  -o ${SPACK_AREA}/spack-${SPACK_VERSION} \
  ${base_release_arg}


cd $SPACK_AREA

spack spec --reuse ${DET}daq@${RELEASE_TAG}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 |  tee $SPACK_AREA/spec_${DET}daq_log.txt
spack install --reuse ${DET}daq@${RELEASE_TAG}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64

if [[ "$DET" == "fd" || "$DET" == "nd" ]]; then
    # Generate pyvenv_requirements.txt
    spack load ${DET}daq@${RELEASE_TAG}
    /usr/bin/python3 $DAQ_RELEASE_REPO/scripts/spack/make-release-repo.py \
        -o ${SPACK_AREA}/../ \
        --pyvenv-requirements \
        -i $DAQ_RELEASE_REPO/configs/${DET}daq/${DET}daq-develop/release.yaml

    python -m venv --prompt dbt ${SPACK_AREA}/../.venv
    source ${SPACK_AREA}/../.venv/bin/activate

    python -m pip install -r ${SPACK_AREA}/../pyvenv_requirements.txt

    pushd ${RELEASE_DIR}
    cp $DAQ_RELEASE_REPO/configs/${DET}daq/${DET}daq-develop/dbt-build-order.cmake .
    tar zcf venv.tar.gz .venv/
    popd

fi

spack clean -a
