
export SPACK_VERSION=0.20.0
export GCC_VERSION=12.1.0
export EXT_VERSION=v2.0
export SPACK_EXTERNALS=/cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-${EXT_VERSION}/spack-$SPACK_VERSION-gcc-$GCC_VERSION

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
  
   release_type=$1

   if [[ -z $release_type || ("$release_type" != "base" && "$release_type" != "det") ]]; then
       echo "You need to pass daqify_spack_environment either \"base\" or \"det\" to specify whether to set up for a base release or a detector release" >&2
      return 4 
   fi

   if [[ -z $SPACK_VERSION || -z $SPACK_AREA || -z $SPACK_EXTERNALS ]]; then
       echo "At least one of the environment variables you need for daqify_spack_environment isn't set; returning..." >&2
       return 2
   fi

   if [[ "$release_type" == "det" && ( -z $BASE_RELEASE_TAG || -z $BASE_SPACK_AREA ) ]]; then
       echo "At least one of the environment variables you need for daqify_spack_environment isn't set; returning..." >&2
       return 2
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

   if [[ "$release_type" == "base" ]]; then

   cat <<EOF > $SPACK_ROOT/etc/spack/defaults/repos.yaml 
          repos:
            - ${SPACK_AREA}/spack-${SPACK_VERSION}/spack-repo
            - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-externals
            - \$spack/var/spack/repos/builtin
EOF



   cat <<EOF  >> $SPACK_ROOT/etc/spack/defaults/upstreams.yaml  
          upstreams:
            spack-externals:
              install_tree: ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/opt/spack
EOF

    elif [[ "$release_type" == "det" ]]; then

   cat <<EOF > $SPACK_ROOT/etc/spack/defaults/repos.yaml 
          repos:
            - ${SPACK_AREA}/spack-${SPACK_VERSION}/spack-repo
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

    cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/linux/compilers.yaml \
      $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/linux/
  
    spack compiler list

    \cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml  $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml
    \cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/modules.yaml  $SPACK_AREA/spack-${SPACK_VERSION}/etc/spack/defaults/modules.yaml

    sed -i 's/host_compatible: true/host_compatible: false/g' $SPACK_ROOT/etc/spack/defaults/concretizer.yaml
}

function tar_and_stage_release() {

    tarfile=$1

    tar zcf $tarfile ${RELEASE_TAG}
    tardir=$GITHUB_WORKSPACE/tarballs_for_upload
    mkdir -p $tardir
    rm -f $tardir/$tarfile
    mv $tarfile $tardir/
}
