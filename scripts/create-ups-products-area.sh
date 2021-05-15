#!/bin/bash

target_products_dir=$PWD/products

while getopts ":p:t:h" opt; do
  case ${opt} in
    t )
       target_products_dir=$OPTARG
       ;;
    h )
      echo "Usage:"
      echo "    create-ups-products-area.sh  -h Display this help message."
      echo "    [-t] <new products dir>"
      exit 0
      ;;
   \? )
     echo "Invalid Option: -$OPTARG" 1>&2
     exit 1
     ;;
  esac
done

if [ -d $target_products_dir ]; then
    echo "Error: Directory $target_products_dir exists, it must be empty, exit now."
    exit 1
fi

mkdir -p $target_products_dir
pushd $target_products_dir
rm -f ups-products-area.tar.bz2
wget https://github.com/DUNE-DAQ/daq-release/raw/develop/misc/ups-products-area.tar.bz2
tar xf ups-products-area.tar.bz2
rm -f ups-products-area.tar.bz2
popd

echo "Info: New UPS products area has been created at: $target_products_dir"
echo "Info: to use it, run 'source $target_products_dir/setup'"
