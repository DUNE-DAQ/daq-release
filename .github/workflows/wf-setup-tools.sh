
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
