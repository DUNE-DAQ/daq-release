#!/bin/bash

products_dir="/home/dingpf/cvmfs_dune/dunedaq/DUNE/products"
#products_dir="/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products"
release_name="dunedaq-v2.6.0"
#release_name="dunedaq-develop"
tarball="dunedaq-${release_name}"
ups_list_file="NOTSET" # Example can be found at daq-release/configs/dunedaq-v2.0.0.release
release_config_dir="NOTSET" # Example can be found at daq-release/configs/dunedaq-v2.0.0

function copy_over_products {
  exclude_list="gcc clang boost"
  prd_list_name=$1[@]
  prd_list=("${!prd_list_name}")
  rel_prd_path=$2
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]}
    prod_version=${iprd_arr[1]}
    if [[ $exclude_list =~ (^|[[:space:]])"$prod_name"($|[[:space:]]) ]]; then
        continue;
    fi
    mkdir ${prod_name}
    pushd ${prod_name}
    [[ -d "${products_dir}/${prod_name}/current.chain" ]] && cp -pr ${products_dir}/${prod_name}/current.chain .
    cp -pr ${products_dir}/${prod_name}/${prod_version} .
    cp -pr ${products_dir}/${prod_name}/${prod_version}.version .
    popd
  done
}

function fix_cvmfs_path_cmake {
  for i in `find $1 -name "*\.cmake"`; do
    if grep -q "libtbb.so" $i; then
      sed -i -E "s/\/([^;]+)\/libtbb.so/\$ENV{TBB_LIB}\/libtbb.so/g" $i
    fi
    if grep -q "libcetlib_except.so" $i; then
      sed -i -E "s/\/([^;]+)\/libcetlib_except.so/\$ENV{CETLIB_EXCEPT_LIB}\/libcetlib_except.so/g" $i
    fi
    if grep -q "libcetlib.so" $i; then
      sed -i -E "s/\/([^;]+)\/libcetlib.so/\$ENV{CETLIB_LIB}\/libcetlib.so/g" $i
    fi
  done
}

function fix_cvmfs_path_dbt_settings {
  for i in `find $1 -name "dbt-settings.sh"`; do
    if grep -q "cvmfs" $i; then
      sed -i "s/\/cvmfs\/dune.opensciencegrid.org\/dunedaq\/DUNE\/releases/\/scratch\/dunedaq-local-releases/g" $i
    fi
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
    r )
       release_name=$OPTARG
       ;;
    t )
       tarball=$OPTARG
       ;;
    h )
      echo "Usage:"
      echo "    copy-over-products.sh  -h Display this help message."
      echo "    <-f> <release_config_dir>"
      echo "    [-P] <products_dir>"
      echo "    [-r] <release_name>"
      echo "    [-t] <tarball_prefix>"
      echo "Example: ./create-release-tarballs.sh -f ./daq-release/configs/dunedaq-v2.2.0 -P /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products -t ./dunedaq-v2.2.0"
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
  echo "[Error]: E.g. daq-release/configs/dunedaq-v2.0.0/release_manifest.sh"
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
  fix_cvmfs_path_dbt_settings $tmp_dir/$release_name
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

tarball_ext="${tarball}_external_pkgs.tar.gz"
tarball_daq="${tarball}_daq_pkgs.tar.gz"
if ! [ -w $(dirname "${tarball}") ]; then
  echo "[Error]: The path to tarball is not writable: $tarball"
fi

echo "[Info]: Creating release tarball containing external packages only: ${tarball_ext}"
echo "[Info]: Creating additional release tarball with DAQ packages: ${tarball_daq}"
echo "[Info]: UPS list file: ${ups_list_file}"
echo "[Info]: Products directory: ${products_dir}"
echo "[Info]: Release name: ${release_name}"


pushd $tmp_dir/$release_name

mkdir externals
pushd externals
cp -pr $products_dir/.updfiles .
cp -pr $products_dir/.upsfiles .
mkdir ups
pushd ups
ups_string=(${dune_ups[0]})
cp -pr  ${products_dir}/${ups_string[0]}/${ups_string[1]} .
cp -pr  ${products_dir}/${ups_string[0]}/${ups_string[1]}.version .
cp -pr  ${products_dir}/${ups_string[0]}/current.chain .
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
copy_over_products dune_externals
popd

pushd packages
copy_over_products dune_packages
popd

popd


fix_cvmfs_path_cmake $tmp_dir/$release_name/packages
mv $tmp_dir/$release_name/packages $tmp_dir
tar -zcvf ${tarball_ext} -C $tmp_dir $release_name
mv $tmp_dir/packages $tmp_dir/$release_name
tar -zcvf ${tarball_daq} -C $tmp_dir $release_name/packages

echo "[Info]: Tarball -- $tarball_ext and $tarball_daq -- has been created."
echo "[Info]: It can be expanded with 'tar -C RELEASE_AREA -zxf $tarball_ext'"
echo "[Info]: In addition, you will need gcc, boost, and clang from scisoft to complete the release."
echo "[Info]: To do so, run the following:"
echo "[Info]: ======================== Code snippet begins ====================="
echo "# create an empty directory for hosting the release;"
echo "export DUNE_DAQ_RELEASE_DIR='./dunedaq-releases'"
echo "mkdir -p \$DUNE_DAQ_RELEASE_DIR"
echo "pushd \$DUNE_DAQ_RELEASE_DIR"
echo "tar zxf $tarball_ext # Or curl/wget to retrieve the tarball first if not having it locally yet"
echo "# get additional UPS products needed: [gcc, boost, clang]; due to their size, those products"
echo "# are not included in the release tarball."
echo "pushd $release_name/externals/"
echo "curl https://scisoft.fnal.gov/scisoft/packages/boost/v1_73_0/boost-1.73.0-sl7-x86_64-e19-prof.tar.bz2|tar xj"
echo "curl https://scisoft.fnal.gov/scisoft/packages/gcc/v8_2_0/gcc-8.2.0-sl7-x86_64.tar.bz2|tar xj"
echo "curl https://scisoft.fnal.gov/scisoft/packages/clang/v7_0_0rc3/clang-7.0.0rc3-sl7-x86_64.tar.bz2|tar xj"
echo "popd; popd"
echo "[Info]: ======================== Code snippet ends == ====================="
echo "[Info]: One laste thing before the release can be used, edit the 'dune_product_dirs' in \$DUNE_DAQ_RELEASE_DIR/$release_name/dbt-settings.sh using the actual path on the deployment node."

rm -rf $tmp_dir
