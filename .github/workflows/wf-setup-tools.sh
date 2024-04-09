
if [[ -z $CORE_RELEASE_DIR || -z $FULL_RELEASE_DIR || -z $OS ]]; then
    echo "You need to define the release's CORE_RELEASE_DIR, FULL_RELEASE_DIR and OS variables for this script to source correctly; returning..." >&2
    return 1
fi

if [[ $CORE_RELEASE_DIR =~ "/release" ]]; then
    export RELEASE_TYPE="frozen"
elif [[ $CORE_RELEASE_DIR =~ "/candidate" ]]; then
    export RELEASE_TYPE="candidate"
elif [[ $CORE_RELEASE_DIR =~ "_PROD4_" ]]; then
    export RELEASE_TYPE="production_v4"
else
    export RELEASE_TYPE="nightly"
fi

echo "Deduced release type \"${RELEASE_TYPE}\" from the name of the core release directory"

export CORE_RELEASE_TAG=$( basename $CORE_RELEASE_DIR )
echo "Assuming core release tag is $CORE_RELEASE_TAG (i.e. the same name as the lowest-level directory in the path ${CORE_RELEASE_DIR})"

export FULL_RELEASE_TAG=$( basename $FULL_RELEASE_DIR )
echo "Assuming full release tag is $FULL_RELEASE_TAG (i.e. the same name as the lowest-level directory in the path ${FULL_RELEASE_DIR})"

if [[ $OS == almalinux9 && $RELEASE_TYPE == production_v4 ]]; then  # Alma9 v4 production nightly, externals v2.0
    export EXT_VERSION=v2.0
elif [[ $OS == almalinux9 && $RELEASE_TYPE == nightly ]]; then      # Alma9 v5 development nightly, externals v2.1
    export EXT_VERSION=v2.1
elif [[ $OS == almalinux9 && $FULL_RELEASE_TAG =~ "v4." ]]; then        # Alma9 v4 candidate or frozen, externals v2.0
    export EXT_VERSION=v2.0
elif [[ $OS == almalinux9 ]]; then                                  # Alma9 v5 candidate or frozen, externals v2.1 
    export EXT_VERSION=v2.1
elif [[ $OS == scientific7 ]]; then                                 # SL7 _any_, externals v1.1
    export EXT_VERSION=v1.1
else
    echo "Environment variable \"OS\" set to unknown operating system \"$OS\"; returning..." >&2
    return 5
fi

echo "Using externals version $EXT_VERSION"

export SPACK_VERSION=0.20.0
export SPACK_EXTERNALS=/cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-${EXT_VERSION}

export FULL_SPACK_AREA=$FULL_RELEASE_DIR
export CORE_SPACK_AREA=$CORE_RELEASE_DIR

function get_spack() {

  if [[ -z $SPACK_VERSION ]]; then
    echo "SPACK_VERSION not defined!" >&2
    return 1
  fi

  wget https://github.com/spack/spack/archive/refs/tags/v${SPACK_VERSION}.tar.gz || return 2
  tar xf v${SPACK_VERSION}.tar.gz 
  ln -s spack-${SPACK_VERSION} spack-installation
  rm -f v${SPACK_VERSION}.tar.gz
}

function daqify_spack_environment() {
  
   release_level=$1

   if [[ -z $release_level || ("$release_level" != "core" && "$release_level" != "full") ]]; then
       echo "You need to pass daqify_spack_environment either \"core\" or \"full\" to specify whether to set up for a core release or a full release" >&2
      return 4 
   fi

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

   if [[ "$release_level" == "core" ]]; then

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
  - ${FULL_SPACK_AREA}/spack-${SPACK_VERSION}/spack-repo
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

    if [[ "$release_level" == "core" ]]; then
        SPACK_AREA=$CORE_SPACK_AREA
    else
        SPACK_AREA=$FULL_SPACK_AREA
    fi

    cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/linux/compilers.yaml \
      $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/linux/
  
    spack compiler list

    \cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml  $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml
    \cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/modules.yaml  $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/modules.yaml

    sed -i 's/host_compatible: true/host_compatible: false/g' $SPACK_ROOT/etc/spack/defaults/concretizer.yaml
}

# get_release_yaml will either take "core" as an argument and deduce
# the needed release.yaml file, or it will take the type of the full
# release and do it (e.g., "nddaq")

function get_release_yaml() {

    release_info=$1

    version=""
    if [[ $RELEASE_TYPE == "candidate" || $RELEASE_TYPE == "frozen" ]]; then
        if [[ $release_info == "core" ]]; then
            version=$( echo $CORE_RELEASE_TAG | sed -r 's/.*(v[0-9]+\.[0-9]+\.[0-9]+).*/\1/' )  
        else
            version=$( echo $FULL_RELEASE_TAG | sed -r 's/.*(v[0-9]+\.[0-9]+\.[0-9]+).*/\1/' )
	fi
    fi

    if [[ $release_info == "core" ]]; then
        
	if [[ $RELEASE_TYPE == "candidate" || $RELEASE_TYPE == "frozen" ]]; then
            echo -n "configs/dunedaq/dunedaq-${version}/release.yaml"
	elif [[ $RELEASE_TYPE == "production_v4" ]]; then
            echo -n "configs/dunedaq/dunedaq-production_v4/release.yaml"
	else
	    echo -n "configs/dunedaq/dunedaq-develop/release.yaml"
        fi

    else

	if [[ $RELEASE_TYPE == "candidate" || $RELEASE_TYPE == "frozen" ]]; then
            echo -n "configs/${release_info}/${release_info}-${version}/release.yaml"
	elif [[ $RELEASE_TYPE == "production_v4" ]]; then
            echo -n "configs/${release_info}/${release_info}-production_v4/release.yaml"
        else
            echo -n "configs/${release_info}/${release_info}-develop/release.yaml"
        fi
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



