
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

function bad_function() {

  echo "About to do something that will cause an error" >& 2
  perl -e 'print(5.0/0)'  
   
  echo "About to drop a false-bomb" >&2
  false
}

function daqify_spack_environment() {

   if [[ -z $SPACK_ROOT ]]; then
       echo "Spack doesn't appear to have been set up; returning..." >&2
       return 1
   fi

   if [[ -z $SPACK_VERSION || -z $SPACK_RELEASE || -z $SPACK_EXTERNALS ]]; then
       echo "At least one of the environment variables you need for daqify_spack_environment isn't set; returning..." >&2
       return 2
   fi

   if [[ ! -e spack-${SPACK_VERSION}/share/spack/setup-env.sh ]]; then
       echo "Can't find spack-${SPACK_VERSION}/share/spack/setup-env.sh; you may be calling daqify_spack_environment from the wrong directory ($PWD). Contents are as follows:"
       ls -ltr
       return 3
   fi

   source spack-${SPACK_VERSION}/share/spack/setup-env.sh

   echo "*********** spack arch ************ "
   spack arch

   cat <<EOF > $SPACK_ROOT/etc/spack/defaults/repos.yaml 
          repos:
            - ${SPACK_RELEASE}/spack-${SPACK_VERSION}/spack-repo
            - ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/spack-repo-externals
            - \$spack/var/spack/repos/builtin
EOF

   spack repo list

   cat <<EOF  >> $SPACK_ROOT/etc/spack/defaults/upstreams.yaml  
          upstreams:
            spack-externals:
              install_tree: ${SPACK_EXTERNALS}/spack-${SPACK_VERSION}/opt/spack
EOF

    cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/linux/compilers.yaml \
      $SPACK_RELEASE/spack-${SPACK_VERSION}/etc/spack/defaults/linux/
  
    spack compiler list

    \cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml  $SPACK_RELEASE/spack-${SPACK_VERSION}/etc/spack/defaults/config.yaml
    \cp $SPACK_EXTERNALS/spack-${SPACK_VERSION}/etc/spack/defaults/modules.yaml  $SPACK_RELEASE/spack-${SPACK_VERSION}/etc/spack/defaults/modules.yaml

    sed -i 's/host_compatible: true/host_compatible: false/g' $SPACK_ROOT/etc/spack/defaults/concretizer.yaml
}
