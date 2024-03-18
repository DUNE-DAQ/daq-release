
if [[ -z $BASE_RELEASE_DIR || -z $DET_RELEASE_DIR || -z $OS ]]; then
    echo "You need to define the release's BASE_RELEASE_DIR, DET_RELEASE_DIR and OS variables for this script to source correctly; returning..." >&2
    return 1
fi

if [[ $BASE_RELEASE_DIR =~ "_DEV_" ]]; then
    export RELEASE_TYPE="nightly"
elif [[ $BASE_RELEASE_DIR =~ "_PROD4_" ]]; then
    export RELEASE_TYPE="production_v4"
elif [[ $BASE_RELEASE_DIR =~ "/candidate" ]]; then
    export RELEASE_TYPE="candidate"
elif [[ $BASE_RELEASE_DIR =~ "/release" ]]; then
    export RELEASE_TYPE="frozen"
else
    echo "Provided BASE_RELEASE_DIR \"${BASE_RELEASE_DIR}\" appears nonstandard and cannot be parsed; returning..." >&2
    return 10
fi

echo "Deduced release type \"${RELEASE_TYPE}\" from the name of the base release directory"

export BASE_RELEASE_TAG=$( basename $BASE_RELEASE_DIR )
echo "Assuming base release tag is $BASE_RELEASE_TAG (i.e. the same name as the lowest-level directory in the path ${BASE_RELEASE_DIR})"

export DET_RELEASE_TAG=$( basename $DET_RELEASE_DIR )
echo "Assuming detector release tag is $DET_RELEASE_TAG (i.e. the same name as the lowest-level directory in the path ${DET_RELEASE_DIR})"

if [[ $OS == almalinux9 && $RELEASE_TYPE == nightly ]]; then
    export EXT_VERSION=v2.1
elif [[ $OS == almalinux9 && $RELEASE_TYPE == production_v4 ]]; then
    export EXT_VERSION=v2.0
elif [[ $OS == scientific7 ]]; then
    export EXT_VERSION=v1.1
else
    echo "Environment variable \"OS\" set to unknown operating system \"$OS\"; returning..." >&2
    return 5
fi

echo "Using externals version $EXT_VERSION"

export SPACK_VERSION=0.20.0
export SPACK_EXTERNALS=/cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-${EXT_VERSION}

export DET_SPACK_AREA=$DET_RELEASE_DIR
export BASE_SPACK_AREA=$BASE_RELEASE_DIR

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

   if [[ -z $release_level || ("$release_level" != "base" && "$release_level" != "det") ]]; then
       echo "You need to pass daqify_spack_environment either \"base\" or \"det\" to specify whether to set up for a base release or a detector release" >&2
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

   if [[ "$release_level" == "base" ]]; then

   cat <<EOF > $SPACK_ROOT/etc/spack/defaults/repos.yaml 
repos:
  - ${BASE_SPACK_AREA}/spack-${SPACK_VERSION}/spack-repo
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOF



   cat <<EOF  >> $SPACK_ROOT/etc/spack/defaults/upstreams.yaml  
upstreams:
  spack-externals:
    install_tree: ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/opt/spack
EOF

    elif [[ "$release_level" == "det" ]]; then

   cat <<EOF > $SPACK_ROOT/etc/spack/defaults/repos.yaml 
repos:
  - ${DET_SPACK_AREA}/spack-${SPACK_VERSION}/spack-repo
  - ${BASE_SPACK_AREA}/spack-${SPACK_VERSION}/spack-repo
  - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-externals
  - \$spack/var/spack/repos/builtin
EOF

   cat <<EOF  >> $SPACK_ROOT/etc/spack/defaults/upstreams.yaml  
upstreams:
  ${BASE_RELEASE_TAG}:
    install_tree: ${BASE_SPACK_AREA}/spack-${SPACK_VERSION}/opt/spack
  spack-externals:
    install_tree: ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/opt/spack
EOF

    fi

    spack repo list

    if [[ "$release_level" == "base" ]]; then
        SPACK_AREA=$BASE_SPACK_AREA
    else
        SPACK_AREA=$DET_SPACK_AREA
    fi

    cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/linux/compilers.yaml \
      $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/linux/
  
    spack compiler list

    \cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml  $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml
    \cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/modules.yaml  $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/modules.yaml

    sed -i 's/host_compatible: true/host_compatible: false/g' $SPACK_ROOT/etc/spack/defaults/concretizer.yaml
}

function get_release_yaml() {

    release_level=$1

    if [[ -z $release_level || ("$release_level" != "base" && "$release_level" != "fd" && "$release_level" != "nd") ]]; then
        echo "You need to pass \"get_release_yaml\" either \"base\", \"fd\" or \"nd\" to get it to find the release's YAML file. Returning..." >&2
        return 1
    fi

    version=""
    if [[ $RELEASE_TYPE == "candidate" || $RELEASE_TYPE == "frozen" ]]; then
        if [[ $release_level == "base" ]]; then
            version=$( echo $BASE_RELEASE_TAG | sed -r 's/.*(v[0-9]+\.[0-9]+\.[0-9]+).*/\1/' )  
        else
            version=$( echo $DET_RELEASE_TAG | sed -r 's/.*(v[0-9]+\.[0-9]+\.[0-9]+).*/\1/' )
	fi
    fi

    if [[ $release_level == "base" ]]; then

        if [[ $RELEASE_TYPE == "nightly" ]]; then
            echo -n "configs/dunedaq/dunedaq-develop/release.yaml"
        elif [[ $RELEASE_TYPE == "production_v4" ]]; then
            echo -n "configs/dunedaq/dunedaq-production_v4/release.yaml"
	elif [[ $RELEASE_TYPE == "candidate" || $RELEASE_TYPE == "frozen" ]]; then
            echo -n "configs/dunedaq/dunedaq-${version}/release.yaml"
        fi

    else

        if [[ $RELEASE_TYPE == "nightly" ]]; then
            echo -n "configs/${release_level}daq/${release_level}daq-develop/release.yaml"
        elif [[ $RELEASE_TYPE == "production_v4" ]]; then
            echo -n "configs/${release_level}daq/${release_level}daq-production_v4/release.yaml"
        elif [[ $RELEASE_TYPE == "candidate" || $RELEASE_TYPE == "frozen" ]]; then
            echo -n "configs/${release_level}daq/${release_level}daq-${version}/release.yaml"
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



