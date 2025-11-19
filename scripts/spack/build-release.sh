#!/bin/bash

# JCF, Nov-18-2025: The basic model here is that a Spack umbrella
# package which people use (e.g., fddaq) always depends on coredaq,
# which is an umbrella package encompassing our core packages (ers,
# logging, appfwk, etc.). This script can build either coredaq or an
# end-product "target" umbrella package; as you might expect, its
# behavior differs depending on which of the two options is passed.

if (( $# < 4 || $# > 6 )); then
    echo "Usage: $( basename $0 ) <desired coredaq release directory>
                        <desired target release directory>
                        <build name (coredaq, fddaq, etc.)>
                        <OS>
                        (optional default repo branch (not used in candidate or stable builds))
                        (optional buildcache directory)" >&2
    exit 1
fi

export CORE_RELEASE_DIR=$1
export TARGET_RELEASE_DIR=$2
export NAME=$3
export OS=$4

export DEFAULT_BRANCH="develop"
if [[ -n $5 ]]; then
    export DEFAULT_BRANCH=$5
fi

if [[ -n $6 ]]; then
    export BUILDCACHE_DIR=$6
fi

if [[ -n $BUILDCACHE_DIR && ! -d $BUILDCACHE_DIR ]]; then
    echo "A buildcache directory \"$BUILDCACHE_DIR\" was provided but it's not found on "$( hostname )"; exiting..." >&2
    exit 4
fi

export DAQ_RELEASE_REPO=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../..
. $DAQ_RELEASE_REPO/scripts/spack/release-setup-tools.sh || exit 3

if [[ $NAME == "coredaq" ]]; then
    export SPACK_AREA=$CORE_SPACK_AREA
else
    export SPACK_AREA=$TARGET_SPACK_AREA
fi

mkdir -p $SPACK_AREA
cd $SPACK_AREA
get_spack

daqify_spack_environment $NAME
release_yaml=$( get_release_yaml "$NAME" )


if [[ "$NAME" == "coredaq" ]]; then
  possible_core_release_arg=""
  export RELEASE_TAG=$CORE_RELEASE_TAG
else
  possible_core_release_arg="--core-release ${CORE_RELEASE_TAG}"
  export RELEASE_TAG=$TARGET_RELEASE_TAG
fi

if [[ $RELEASE_TYPE == "nightly" ]]; then
  possible_branch_arg="-b "${DEFAULT_BRANCH}
else
  possible_branch_arg=""
fi

cd $DAQ_RELEASE_REPO

spack_template_dir=spack-repos/${NAME}-repo-template

cmd="python3 scripts/spack/make-release-repo.py -u \
  -i ${release_yaml} \
  -t $spack_template_dir \
  -r ${RELEASE_TAG} \
  -o ${SPACK_AREA}/spack-${SPACK_VERSION} \
  ${possible_core_release_arg} \
  ${possible_branch_arg}"

echo $cmd
$cmd || exit 5

cd $SPACK_AREA

spack clean -m 

if [[ -n $BUILDCACHE_DIR ]]; then

    mirror_name=$( basename ${BUILDCACHE_DIR%/} )-mirror
    
    spack mirror list
    if [[ -z $( spack mirror list | grep $mirror_name ) ]]; then
	spack mirror add --unsigned $mirror_name file://$BUILDCACHE_DIR || exit 21
    else
	echo "Already have $mirror_name available; won't add it"
    fi
fi

# JCF, Mar-3-2025

# Supplying a buildcache directory to build-release.sh is something
# that would be done at the command line, typically in a situation
# where speed as opposed to record-keeping is a priority, so we're
# skipping the logging of a spec in that case

if [[ -z $BUILDCACHE_DIR ]]; then
    spack spec -l --reuse ${NAME}@${RELEASE_TAG}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=linux-${OS}-x86_64 > $SPACK_AREA/spec_${NAME}_log.txt 2>&1
    retval=$?

    cat $SPACK_AREA/spec_${NAME}_log.txt 

    if [[ $retval != 0 ]]; then
	exit 20
    fi
fi

build_dbe=false
if [[ $NAME == "coredaq" && -z $BUILDCACHE_DIR ]]; then
    spack spec -l --reuse dbe%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=linux-${OS}-x86_64 > $SPACK_AREA/spec_dbe_log.txt 2>&1
    retval=$?    

    cat $SPACK_AREA/spec_dbe_log.txt

    if [[ $retval == 0 ]]; then
	build_dbe=true
    else
	build_dbe=false
        echo "Building dbe does not appear to be possible. Will exit..."
	exit 12
    fi
elif [[ $NAME == "coredaq" ]]; then
    build_dbe=true  # Any buildcache provided will be assumed to contain dbe
fi

attempt=1
max_attempts=3
while true; do
    echo " --- ${NAME} build attempt number $attempt of $max_attempts --- "

    spack install --reuse ${NAME}@${RELEASE_TAG}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=linux-${OS}-x86_64 +dev 2>&1 | tee ${NAME}_build_spack_install.log || true
    spack_install_exit_code=${PIPESTATUS[0]}

    if [[ $spack_install_exit_code -eq 0 ]]; then
        echo "Build succeeded on attempt number $attempt"
        break
    else 
        echo "Spack has exited with code $spack_install_exit_code. Checking if this is a retryable error..."
    fi
    if grep -qi "==> Error: FetchError: All fetchers failed" ${NAME}_build_spack_install.log; then
        echo "Attempt $attempt failed due to a FetchError."
        if [[ $attempt -lt $max_attempts ]]; then 
            echo "Retrying..."
        fi
    else
        echo "Build failed with a non-retryable exit code. Exiting..."
        exit $spack_install_exit_code
    fi
    if [[ $attempt -ge $max_attempts ]]; then
        echo "All retry attempts failed due to FetchError. Exiting."
        exit 111
    fi
    attempt=$((attempt + 1))
done

# JCF, Feb-4-2025: since fddaq~dev is a subset of the just-now
# installed fddaq+dev, I don't think network timeouts from Spack
# installing new packages should be a failure mode (see
# https://github.com/DUNE-DAQ/daq-release/pull/423 for more)

spack install --reuse ${NAME}@${RELEASE_TAG}%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=linux-${OS}-x86_64 ~dev || exit 7

if $build_dbe; then
    dbe_attempt=1
    max_dbe_build_attempts=3
    while true; do
        echo " --- dbe build attempt number $dbe_attempt of $max_dbe_build_attempts --- "

	spack install --reuse dbe%gcc@${GCC_VERSION} build_type=RelWithDebInfo arch=linux-${OS}-x86_64 | tee dbe_build_spack_install.log || true
        spack_install_dbe_exit_code=${PIPESTATUS[0]}
        if [[ $spack_install_dbe_exit_code -eq 0 ]]; then
            echo "dbe build succeeded on attempt number $dbe_attempt"
            break
        fi
        if grep -qi "==> Error: FetchError: All fetchers failed" dbe_build_spack_install.log; then
            echo "Attempt $dbe_attempt failed due to a FetchError."
        else
            echo "Build failed with a non-retryable exit code. Exiting..."
            exit $spack_install_dbe_exit_code
        fi
        if [[ $dbe_attempt -ge $max_dbe_build_attempts ]]; then
            echo "All retry attempts failed due to FetchError. Exiting."
            exit 111
        fi
        dbe_attempt=$((dbe_attempt + 1))
    done
fi

if [[ "$NAME" != "coredaq" ]]; then
    # Generate pyvenv_requirements.txt
    spack load ${NAME}@${RELEASE_TAG} +dev || exit 9

    cd $DAQ_RELEASE_REPO
    cmd="/usr/bin/python3 scripts/spack/make-release-repo.py \
        -o ${SPACK_AREA} \
        --pyvenv-requirements \
        -i ${release_yaml}"

    echo $cmd
    $cmd || exit 8

    python -m venv --prompt dbt ${SPACK_AREA}/.venv
    source ${SPACK_AREA}/.venv/bin/activate

    python -m pip install -r ${SPACK_AREA}/pyvenv_requirements.txt || exit 11

    pushd $TARGET_RELEASE_DIR
    cp $DAQ_RELEASE_REPO/$( dirname $release_yaml )/dbt-build-order.cmake .
    tar zcf venv.tar.gz .venv/
    popd
    
fi

echo "Files in $SPACK_AREA :"
ls -ltr $SPACK_AREA

spack clean -a
