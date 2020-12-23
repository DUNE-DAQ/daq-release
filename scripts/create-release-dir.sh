#!/bin/bash

products_dir="/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products"
release_dir="/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/releases-tmp"
release_name="dunedaq-v2.0.0"
#release_name="dunedaq-develop"
tarball="cvmfs_${release_name}.tar.gz"
ups_list_file="NOTSET" # Example can be found at daq-release/configs/dunedaq-v2.0.0.release

function create_products_link {
  prd_list_name=$1[@]
  prd_list=("${!prd_list_name}")
  rel_prd_path=$2
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]}
    prod_version=${iprd_arr[1]}
    mkdir ${prod_name}
    pushd ${prod_name}
    ln -s ../../$rel_prd_path/${prod_name}/${prod_version} .
    popd
  done
}

while getopts ":f:P:R:r:t:h" opt; do
  case ${opt} in
    f )
       ups_list_file=$OPTARG
       ;;
    P )
       products_dir=$OPTARG
       ;;
    R )
       release_dir=$OPTARG
       ;;
    r )
       release_name=$OPTARG
       ;;
    t )
       tarball=$OPTARG
       ;;
    h )
      echo "Usage:"
      echo "    create-release-dir.sh  -h Display this help message."
      echo "    <-f> <ups_list_file>"
      echo "    [-P] <products_dir>"
      echo "    [-R] <release_dir>"
      echo "    [-r] <release_name>"
      echo "    [-t] <tarball_name>"
      exit 0
      ;;
   \? )
     echo "Invalid Option: -$OPTARG" 1>&2
     exit 1
     ;;
  esac
done


shift $((OPTIND -1))

if [[ "$ups_list_file" == "NOTSET" ]]; then
  echo "[Error]: UPS list file must be speficied with option '-f'."
  echo "[Error]: If the dunedaq development environment has been set, you"
  echo "[Error]: can create this file by 'ups active | tail -n +2 > ups_list_file.txt'"
  echo "Exit now..."
  exit 2
else
  source $ups_list_file
fi

if ! [ -w $(dirname "${tarball}") ]; then
  echo "[Error]: The path to tarball is not writable: $tarball"
fi

echo "[Info]: Creating release tarball: ${tarball}"
echo "[Info]: UPS list file: ${ups_list_file}"
echo "[Info]: Products directory: ${products_dir}"
echo "[Info]: Release directory: ${release_dir}"
echo "[Info]: Release name: ${release_name}"


tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)

mkdir $tmp_dir/$release_name
pushd $tmp_dir/$release_name

## Step 1. get `dbt-build-order.cmake, dbt-settings.sh, dunedaq_area.sh and pyvenv_requirements.sh from daq-builtools repo`;


## Step 2. create externals and packages dir

rel_products_dir=`realpath --relative-to="${release_dir%%+(/)}${release_dir:+/}$release_name" $products_dir`

mkdir externals
pushd externals
cp -pr $products_dir/.updfiles .
cp -pr $products_dir/.upsfiles .
mkdir ups
pushd ups
ups_string=(${dune_ups[0]})
ln -s ../../$rel_products_dir/${ups_string[0]}/${ups_string[1]} .
popd # ups

ln -s ${ups_string[0]}/${ups_string[1]}/Linux64bit+3.10-2.17/ups/setup
ln -s ${ups_string[0]}/${ups_string[1]}/Linux64bit+3.10-2.17/ups/setups

cat <<EOF > setups_layout
s_setenv UPS_THIS_DB \$SETUPS_DIR
s_setenv PROD_DIR_PREFIX \$SETUPS_DIR
EOF
popd # externals
cp -pr externals packages

pushd externals
create_products_link dune_externals $rel_products_dir
popd

pushd packages
create_products_link dune_packages $rel_products_dir
popd

popd

tar -zcvf $tarball -C $tmp_dir $release_name

echo "[Info]: Tarball -- $tarball -- has been created."
echo "[Info]: It can be expanded under $release_dir with 'tar -C RELEASE_AREA -zxf $tarball'"

rm -rf $tmp_dir
