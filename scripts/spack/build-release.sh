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
        echo "Building dbe does not appear to be possible. Will exit..."
	exit 12
    fi
fi

#spack install --reuse ${DET}daq@${RELEASE_TAG}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 | tee dunedaq_build_spack_install.log
bash -c "echo '==> Error: FetchError: All fetchers failed'; exit 111" 2>&1 | tee dunedaq_build_spack_install.log
spack_install_exit_code=${PIPESTATUS[0]}

if [[ $spack_install_exit_code -ne 0 ]]; then
    # In case of a transient connection error, try again
    if grep -qi "==> Error: FetchError: All fetchers failed" dunedaq_build_spack_install.log; then
        is_fetch_error=true
        max_attempts=3
        attempt=2
        echo "First build attempt failed due to a FetchError. Will retry up to $max_attempts times."
        while [[ $is_fetch_error && $attempt -le $max_attempts ]]; do
            is_fetch_error=false
            echo " --- Attempt number $attempt of $max_attempts --- "
            echo "Is fetch error = $is_fetch_error"
            echo "spack install exit code = $spack_install_exit_code"
            echo "SPACK_AREA: $SPACK_AREA"
            echo "ls SPACK_AREA:\n $(ls $SPACK_AREA)"
            echo "ls SPACK_AREA/spack-version:\n $(ls $SPACK_AREA/spack-${SPACK_VERSION})"
            echo "ls SPACK_AREA/spack-version/spack-repo:\n $(ls $SPACK_AREA/spack-${SPACK_VERSION}/spack-repo)"
            echo "ls SPACK_AREA/spack-installation:\n $(ls $SPACK_AREA/spack-installation)"
            echo "ls SPACK_AREA/spack-installation/spack-repo:\n $(ls $SPACK_AREA/spack-installation/spack-repo)"
            echo "Diff between $SPACK_AREA/spack-${SPACK_VERSION} and $SPACK_AREA/spack-installation :"
            diff -r $SPACK_AREA/spack-${SPACK_VERSION} $SPACK_AREA/spack-installation
            echo "End diff"
            rm -rf ${SPACK_AREA}/sourcecode
            rm -rf ${SPACK_AREA}/spack-installation/spack-repo
            #rm -rf ${SPACK_AREA}/spack-${SPACK_VERSION}
            #rm -rf spack-${SPACK_VERSION}/spack-repo
            rm -rf spack-${SPACK_VERSION}/default
            spack install --reuse ${DET}daq@${RELEASE_TAG}%gcc@12.1.0 build_type=RelWithDebInfo arch=linux-${OS}-x86_64 | tee dunedaq_build_spack_install.log || true
            #bash -c "echo '==> Error: FetchError: All fetchers failed'; exit 111" | tee dunedaq_build_spack_install.log
            bash -c "echo 'No more error!'; exit 0" 2>&1 | tee dunedaq_build_spack_install.log
            spack_install_exit_code=${PIPESTATUS[0]}
            if [[ $spack_install_exit_code -eq 0 ]]; then
                echo "Build succeeded after attempt number $attempt"
                break
            elif grep -qi "==> Error: FetchError: All fetchers failed" dunedaq_build_spack_install.log; then
                echo "Retry attempt $attempt/$max_attempts failed due to a FetchError."
                is_fetch_error=true
            else
                echo "Build failed with a non-retryable exit code. Exiting..."
                exit $spack_install_exit_code
            fi
            attempt=$((attempt+1))
        done
        if [[ $attempt -gt $max_attempts && $is_fetch_error == true ]]; then
            echo "All retry attempts failed due to FetchError. Exiting."
            exit 111
        fi
    fi
fi

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
