
function download_and_setup_spack() {
    git clone https://github.com/spack/spack.git -b v0.22.0 spack-0.22.0 || exit 20
    . $PWD/spack-0.22.0/share/spack/setup-env.sh || exit 21
}

function get_spack_hash() {
    local package=$1
    local spack_hash=$( spack find -l $package | sed -n -r 's/^(\w{7}) .*/\1/p' )
    if [[ -z $spack_hash ]]; then
	echo "Unable to get hash for $package; exiting..." >&2
	exit 4
    fi
    echo $spack_hash
}


export DAQ_RELEASE_DIR="$(dirname "$(realpath "$0")")"/../..

export EXTERNALS_PACKAGES="$( $DAQ_RELEASE_DIR/scripts/list_packages.py develop externals ) gcc python ninja gdb qt"

if [[ -e $HOME/.spack ]]; then
    echo "Warning: an account-wide Spack directory \"$HOME/.spack\" has been found which *may* interfere with the running of this script." >&2
    #echo "An account-wide Spack directory \"$HOME/.spack\" has been found which could interfere with the running of this script. Exiting..." >&2
    #exit 2
fi

