#!/bin/bash

release_config_dir="/scratch/workdir/daq-release/configs/dunedaq-develop" 
products_dir="/cvmfs/dunedaq.opensciencegrid.org/products"
release_name="N$(date +%y-%m-%d)"
tarball="${release_name}.tar.gz"
upstar_dir="/scratch/tarball"

function create_products_link {
  prd_list_name=$1[@]
  prd_list=("${!prd_list_name}")
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]}
    prod_version=${iprd_arr[1]}
    mkdir ${prod_name}
    pushd ${prod_name}
    [[ -d "${products_dir}/${prod_name}/current.chain" ]] && ln -s ${products_dir}/${prod_name}/current.chain .
    ln -s ${products_dir}/${prod_name}/${prod_version} .
    ln -s ${products_dir}/${prod_name}/${prod_version}.version .
    popd
  done
}

while getopts ":f:P:R:r:t:w:h" opt; do
  case ${opt} in
    f )
       release_config_dir=$OPTARG
       ;;
    P )
       products_dir=$OPTARG
       ;;
    r )
       release_name=$OPTARG
       ;;
    t )
       tarball=$OPTARG
       ;;
    u )
       upstar_dir=$OPTARG
       ;;
    h )
      echo "Usage:"
      echo "    create-release-dir.sh  -h Display this help message."
      echo "    <-f> <release_config_dir>"
      echo "    [-P] <products_dir>"
      echo "    [-r] <release_name>"
      echo "    [-t] <tarball_name>"
      echo "    [-u] <ups tarball dir>"
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
  _dbt_file=$tmp_dir/$release_name/dbt-settings.sh
  cp $release_config_dir/dbt-settings.sh ${_dbt_file}
  sed -i "s/\/scratch\/releases\/dunedaq-develop/\/cvmfs\/dunedaq-development.opensciencegrid.org\/nightly\/${release_name}/g" ${_dbt_file}
  echo "dune_daqpackages=(" >> ${_dbt_file}
  find "/scratch/releases/dunedaq-develop/packages/" -name "*.version" -type d |sed  "s/\/scratch\/releases\/dunedaq-develop\/packages\//   \"/g"|sed 's/\// /g'|sed 's/.version/ -q e19:prof"/g'>>${_dbt_file}
  echo "    )">> ${_dbt_file}
fi

if [ ! -f "$release_config_dir/pyvenv_requirements.txt" ]; then
  echo "[Error]: pyvenv_requirements.txt is missing."
  echo "[Error]: Example can be found in the 'daq-release' repo"
  echo "[Error]: under daq-release/configs/dunedaq-v2.0.0/"
  echo "Exit now..."
  exit 3
else
  cp $release_config_dir/pyvenv_requirements.txt $tmp_dir/$release_name
  sed -i 's/scratch/cvmfs\/dunedaq.opensciencegrid.org/g' $tmp_dir/$release_name/pyvenv_requirements.txt
fi

if ! [ -w $(dirname "${tarball}") ]; then
  echo "[Error]: The path to tarball is not writable: $tarball"
fi

echo "[Info]: Creating release tarball: ${tarball}"
echo "[Info]: Products directory: ${products_dir}"
echo "[Info]: Release name: ${release_name}"


pushd $tmp_dir/$release_name

## Step 1. get `dbt-build-order.cmake, dbt-settings.sh, dunedaq_area.sh and pyvenv_requirements.sh from daq-builtools repo`;


## Step 2. create externals and packages dir


mkdir externals
pushd externals

create_products_link dune_externals
popd

mkdir packages
pushd packages

for i in `ls ${upstar_dir}/*.bz2`; do tar xf $i; done

popd

popd

tar -zcvf $tarball -C $tmp_dir $release_name

echo "[Info]: Tarball -- $tarball -- has been created."
echo "[Info]: It can be expanded under release_dir with 'tar -C RELEASE_AREA -zxf $tarball'"

rm -rf $tmp_dir
