#!/bin/bash

if (( $# < 4 || $# > 5 )); then
    echo "Usage: $( basename $0 ) <desired core release directory> 
                        <desired full release directory> 
                        <target build (\"core\" for core release, anything else for full release)> 
                        <OS (almalinux9 or scientific7)>
                        (optional default repo branch (nightly only, default is develop) )" >&2
    exit 1
fi

export CORE_RELEASE_DIR=$1
export FULL_RELEASE_DIR=$2
export TARGET=$3
export OS=$4

export DEFAULT_BRANCH="develop"
if [[ -n $5 ]]; then
    export DEFAULT_BRANCH=$5
fi

if [[ $OS != "almalinux9" && $OS != "scientific7" ]]; then
    echo "OS needs to be specified either as \"almalinux9\" or \"scientific7\"; exiting..." >&2
    exit 3
fi

export DAQ_RELEASE_REPO=$PWD/$(dirname "$0")/../..
. $DAQ_RELEASE_REPO/.github/workflows/wf-setup-tools.sh || exit 3

if [[ $TARGET == "core" ]]; then
    export SPACK_AREA=$CORE_SPACK_AREA
else
    export SPACK_AREA=$FULL_SPACK_AREA
fi

mkdir -p $SPACK_AREA
cd $SPACK_AREA
get_spack

if [[ "$TARGET" == "core" ]]; then
  daqify_spack_environment core
  release_yaml=$( get_release_yaml "core" )
  base_release_arg=""
  export RELEASE_TAG=$CORE_RELEASE_TAG
else
  daqify_spack_environment full
  release_yaml=$( get_release_yaml "$TARGET" )
  base_release_arg="--base-release ${CORE_RELEASE_TAG}"
  export RELEASE_TAG=$FULL_RELEASE_TAG
fi

if [[ $RELEASE_TYPE == "candidate" || $RELEASE_TYPE == "frozen" ]]; then
    branch_arg=""
else
    branch_arg="-b "${DEFAULT_BRANCH}
fi

cd $DAQ_RELEASE_REPO

if [[ "$TARGET" != "core" ]]; then
    spack_template_dir=spack-repos/${TARGET}-repo-template
else
    spack_template_dir=spack-repos/dunedaq-repo-template
fi

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

if [[ "$TARGET" != "core" ]]; then
    spack spec -l --reuse ${TARGET}@${RELEASE_TAG}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 > $SPACK_AREA/spec_${TARGET}_log.txt 2>&1
    retval=$?
    cat $SPACK_AREA/spec_${TARGET}_log.txt 
else
    echo "FOR ISSUE #361 TESTING PURPOSES, FORCE subset=datautilities"
    spack spec -l --reuse dunedaq@${RELEASE_TAG}%gcc@12.1.0 subset=datautilities build_type=RelWithDebInfo arch=linux-${OS}-x86_64 > $SPACK_AREA/spec_dunedaq_log.txt 2>&1
    retval=$?
    cat $SPACK_AREA/spec_dunedaq_log.txt 
fi

if [[ $retval != 0 ]]; then
    exit 20
fi

build_dbe=false
echo "FOR ISSUE #361 TESTING PURPOSES, IGNORE dbe"
#if [[ $TARGET == "core" ]]; then
if false; then
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

if [[ "$TARGET" != "core" ]]; then
    spack install --reuse ${TARGET}@${RELEASE_TAG}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 || exit 7
else
    echo "FOR ISSUE #361 TESTING PURPOSES, FORCE subset=datautilities"
    spack install --reuse dunedaq@${RELEASE_TAG}%gcc@12.1.0 subset=datautilities build_type=RelWithDebInfo arch=linux-${OS}-x86_64 || exit 7
fi

if $build_dbe; then
    spack install --reuse dbe%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 || exit 8
fi

if [[ "$TARGET" != "core" ]]; then
    # Generate pyvenv_requirements.txt
    spack load ${TARGET}@${RELEASE_TAG} || exit 9

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

    echo "FOR ISSUE #361 TESTING PURPOSES, DISREGARD ANY PIP INSTALLATION ERRORS"
    python -m pip install -r ${SPACK_AREA}/pyvenv_requirements.txt
    #python -m pip install -r ${SPACK_AREA}/pyvenv_requirements.txt || exit 11
    
    pushd $FULL_RELEASE_DIR
    cp $DAQ_RELEASE_REPO/$( dirname $release_yaml )/dbt-build-order.cmake .
    tar zcf venv.tar.gz .venv/
    popd
    
fi

echo "Files in $SPACK_AREA :"
ls -ltr $SPACK_AREA

spack clean -a
