#!/bin/bash

products_dir="/cvmfs/dunedaq.opensciencegrid.org/products"
release_dir="/cvmfs/dunedaq.opensciencegrid.org/releases"
release_name=$1
#release_name="dunedaq-develop"
tarball="cvmfs_${release_name}.tar.gz"
ups_list_file="NOTSET" # Example can be found at daq-release/configs/dunedaq-v2.0.0.release
release_config_dir="NOTSET" # Example can be found at daq-release/configs/dunedaq-v2.0.0

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
    [[ -d "${products_dir}/${prod_name}/current.chain" ]] && ln -s ../../$rel_prd_path/${prod_name}/current.chain .
    ln -s ../../$rel_prd_path/${prod_name}/${prod_version} .
    ln -s ../../$rel_prd_path/${prod_name}/${prod_version}.version .
    popd
  done
}

while getopts ":f:P:R:r:t:h" opt; do
  case ${opt} in
    f )
       ups_list_file=$OPTARG
       release_config_dir=$OPTARG
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
      echo "    <-f> <release_config_dir>"
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

if [[ "$release_config_dir" == "NOTSET" ]]; then
  echo "[Error]: UPS list file must be speficied with option '-f'."
  echo "[Error]: Example can be found in the 'daq-release' repo."
  echo "[Error]: E.g. daq-release/configs/dunedaq-v2.0.0.release"
  echo "Exit now..."
  exit 2
fi

if [ ! -f "$release_config_dir/release_manifest.sh" ]; then
  echo "[Error]: UPS list file must exist in the release config directory."
  echo "[Error]: Example can be found in the 'daq-release' repo."
  echo "[Error]: E.g. daq-release/configs/dunedaq-v2.0.0/release_manifest"
  echo "Exit now..."
  exit 3
else
  source $release_config_dir/release_manifest.sh
fi

tmp_dir=$(mktemp -d -t cvmfs_dunedaq_release_XXXXXXXXXX)
mkdir $tmp_dir/$release_name

if [ ! -f "$release_config_dir/dbt-build-order.cmake" ]; then
  echo "[Error]: dbt-build-order.cmake is missing."
  echo "[Error]: Example can be found in the 'daq-release' repo"
  echo "[Error]: under daq-release/configs/dunedaq-v2.0.0/"
  echo "Exit now..."
  exit 3
else
  cp $release_config_dir/dbt-build-order.cmake $tmp_dir/$release_name
fi

if [ ! -f "$release_config_dir/dbt-settings.sh" ]; then
  echo "[Error]: dbt-settings.sh is missing."
  echo "[Error]: Example can be found in the 'daq-release' repo"
  echo "[Error]: under daq-release/configs/dunedaq-v2.0.0/"
  echo "Exit now..."
  exit 3
else
  cp $release_config_dir/dbt-settings.sh $tmp_dir/$release_name
fi

if [ ! -f "$release_config_dir/pyvenv_requirements.txt" ]; then
  echo "[Error]: pyvenv_requirements.txt is missing."
  echo "[Error]: Example can be found in the 'daq-release' repo"
  echo "[Error]: under daq-release/configs/dunedaq-v2.0.0/"
  echo "Exit now..."
  exit 3
else
  cp $release_config_dir/pyvenv_requirements.txt $tmp_dir/$release_name
fi

if ! [ -w $(dirname "${tarball}") ]; then
  echo "[Error]: The path to tarball is not writable: $tarball"
fi

echo "[Info]: Creating release tarball: ${tarball}"
echo "[Info]: UPS list file: ${ups_list_file}"
echo "[Info]: Products directory: ${products_dir}"
echo "[Info]: Release directory: ${release_dir}"
echo "[Info]: Release name: ${release_name}"


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
ln -s ../../$rel_products_dir/${ups_string[0]}/${ups_string[1]}.version .
ln -s ../../$rel_products_dir/${ups_string[0]}/current.chain .
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
