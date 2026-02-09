
if [[ -z $CORE_RELEASE_DIR || -z $TARGET_RELEASE_DIR || -z $OS ]]; then
    echo "You need to define the release's CORE_RELEASE_DIR, TARGET_RELEASE_DIR and OS variables for this script to source correctly; returning..." >&2
    return 1
fi

if [[ $CORE_RELEASE_DIR =~ "_DEV_" ]]; then
    export RELEASE_TYPE="nightly"
elif [[ $CORE_RELEASE_DIR =~ "/candidate" ]]; then
    export RELEASE_TYPE="candidate"
elif [[ $CORE_RELEASE_DIR =~ "/release" ]]; then
    export RELEASE_TYPE="stable"
else
    echo "Provided CORE_RELEASE_DIR \"${CORE_RELEASE_DIR}\" appears nonstandard and cannot be parsed; returning..." >&2
    return 10
fi

# Deduce what the full umbrella package is based on the name of its repo
# E.g. /cvmfs/dunedaq-development.opensciencegrid.org/nightly/NFD_DEV_240426_A9 => fddaq

# The name of the full umbrella package will match both the needed
# value of the "subset" variant for the coredaq and externals
# packages, as well as the path to the relevant release.yaml files

# E.g., if TARGET gets set to fddaq, that means the
# release.yaml for the relevant variant of the coredaq umbrella
# package is in
#
# ./configs/coredaq/fddaq-develop/
#
# and that the full umbrella package is "fddaq", and it depends on
# "coredaq subset=fddaq", which in turn depends on "externals subset=fddaq"

export TARGET=

if [[ $RELEASE_TYPE == "nightly" ]]; then
    export TARGET_ABBREV=$( echo $TARGET_RELEASE_DIR | sed -r 's!.*/.*(FD|ND|FDDU)_.*!\1!' )

    if [[ -z $TARGET_ABBREV ]]; then
	echo "Unable to intepret abbreviation of full umbrella package from repo path name ${FULL_RELEASE_DIR}; exiting...
" >&2
	exit 4
    fi

    if [[ "$TARGET_ABBREV" == "FD" ]]; then
	export TARGET="fddaq"
    elif [[ "$TARGET_ABBREV" == "ND" ]]; then
	export TARGET="nddaq"
    elif [[ "$TARGET_ABBREV" == "FDDU" ]]; then
	export TARGET="fddatautilities"
    else
	echo "Unknown full umbrella package abbreviation \"$TARGET_ABBREV\"; exiting..." >&2
	exit 5
    fi
else  # candidate or stable build
    export TARGET=$( echo $TARGET_RELEASE_DIR | sed -r 's!.*/([^-]+).*!\1!' )
fi

echo "Deduced full umbrella package as \"${TARGET}\""
echo "Deduced release type \"${RELEASE_TYPE}\" from the name of the coredaq directory"

export CORE_RELEASE_TAG=$( basename $CORE_RELEASE_DIR )
export TARGET_RELEASE_TAG=$( basename $TARGET_RELEASE_DIR )

export EXT_VERSION=v2.3
export GCC_VERSION=14.3.0
export SPACK_VERSION=0.22.0

export SPACK_EXTERNALS=${SPACK_EXTERNALS:-/cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-${EXT_VERSION}}

export TARGET_SPACK_AREA=$TARGET_RELEASE_DIR
export CORE_SPACK_AREA=$CORE_RELEASE_DIR

function get_spack() {

  if [[ -z $SPACK_VERSION ]]; then
    echo "SPACK_VERSION not defined!" >&2
    return 1
  fi

  wget https://github.com/spack/spack/archive/refs/tags/v${SPACK_VERSION}.tar.gz || return 2
  tar xf v${SPACK_VERSION}.tar.gz
  rm -f v${SPACK_VERSION}.tar.gz
  
  ln -s spack-${SPACK_VERSION} spack-installation
  #mkdir spack-${SPACK_VERSION}/default
  #pushd spack-${SPACK_VERSION}/default
  #ln -s ../spack-installation
  #popd
}

function daqify_spack_environment() {
  
   local umbrella_package=$1

   if [[ ! -e spack-${SPACK_VERSION}/share/spack/setup-env.sh ]]; then
       echo "Can't find spack-${SPACK_VERSION}/share/spack/setup-env.sh; you may be calling daqify_spack_environment from the wrong directory ($PWD). Contents are as follows:"
       ls -ltr
       return 3
   fi

   source spack-${SPACK_VERSION}/share/spack/setup-env.sh

   if [[ -z $SPACK_ROOT ]]; then
       echo "Spack doesn't appear to have been set up; returning..." >&2
       return 1
   fi

   echo "*********** spack arch ************ "
   spack arch

   if [[ "$umbrella_package" == "coredaq" ]]; then

   echo "Deleting $SPACK_ROOT/etc/spack/defaults/repos.yaml to remake it"
   rm -f $SPACK_ROOT/etc/spack/defaults/repos.yaml
   cat <<EOF > $SPACK_ROOT/etc/spack/defaults/repos.yaml 
repos:
  - ${CORE_SPACK_AREA}/spack-${SPACK_VERSION}/spack-repo
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOF



   cat <<EOF  >> $SPACK_ROOT/etc/spack/defaults/upstreams.yaml  
upstreams:
  spack-externals:
    install_tree: ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/opt/spack
EOF

    else

   cat <<EOF > $SPACK_ROOT/etc/spack/defaults/repos.yaml 
repos:
  - ${TARGET_SPACK_AREA}/spack-${SPACK_VERSION}/spack-repo
  - ${CORE_SPACK_AREA}/spack-${SPACK_VERSION}/spack-repo
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOF

   cat <<EOF  >> $SPACK_ROOT/etc/spack/defaults/upstreams.yaml  
upstreams:
  ${CORE_RELEASE_TAG}:
    install_tree: ${CORE_SPACK_AREA}/spack-${SPACK_VERSION}/opt/spack
  spack-externals:
    install_tree: ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/opt/spack
EOF

    fi

    spack repo list

    if [[ "$umbrella_package" == "coredaq" ]]; then
        SPACK_AREA=$CORE_SPACK_AREA
    else
        SPACK_AREA=$TARGET_SPACK_AREA
    fi

    cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/linux/compilers.yaml \
      $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/linux/
  
    spack compiler list

    \cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml  $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml
    \cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/modules.yaml  $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/modules.yaml

    sed -i 's/host_compatible: true/host_compatible: false/g' $SPACK_ROOT/etc/spack/defaults/concretizer.yaml
}

function get_release_yaml() {

    local name=$1
    
    if [[ $RELEASE_TYPE == "nightly" ]]; then
	if [[ $name == "coredaq" ]]; then
            echo -n "configs/coredaq/${TARGET}-develop/release.yaml"	    
	else
            echo -n "configs/${TARGET}/${TARGET}-develop/release.yaml"
	fi
    elif [[ $RELEASE_TYPE == "candidate" || $RELEASE_TYPE == "stable" ]]; then
	
	if [[ $name == "coredaq" ]]; then
	    version=$( echo $CORE_RELEASE_TAG | sed -r 's/.*(v[0-9]+\.[0-9]+\.[0-9]+).*/\1/' )  
            echo -n "configs/coredaq/${TARGET}-${version}/release.yaml"
	else
	    version=$( echo $TARGET_RELEASE_TAG | sed -r 's/.*(v[0-9]+\.[0-9]+\.[0-9]+).*/\1/' )
	    echo -n "configs/${TARGET}/${TARGET}-${version}/release.yaml"
	fi
    else
	echo -n "unable/to/find/release.yaml"
    fi
}

function tar_and_stage_release() {

    subdir=$1
    tarfile=${subdir}.tar.gz

    echo "About to run tar zcf $tarfile $subdir in $PWD"
    df
    echo "######################################################################"
    du . -s
    echo "######################################################################"
    du $subdir -s
    echo
    
    tar zcf $tarfile $subdir
    tardir=$GITHUB_WORKSPACE/tarballs_for_upload
    mkdir -p $tardir
    rm -f $tardir/$tarfile
    mv $tarfile $tardir/
}

