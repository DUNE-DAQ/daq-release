#!/bin/bash

outdir=$1

create_products_area () {
  target_products_dir=$1
  mkdir -p $target_products_dir
  pushd $target_products_dir
  ln -s /cvmfs/dunedaq.opensciencegrid.org/products/ups .
  cp -pr /cvmfs/dunedaq.opensciencegrid.org/products/.upsfiles .
  cp -pr /cvmfs/dunedaq.opensciencegrid.org/products/.updfiles .
  echo "s_setenv UPS_THIS_DB $SETUPS_DIR" > setups_layout
  echo "s_setenv PROD_DIR_PREFIX $SETUPS_DIR" >> setups_layout
  ln -s ups/v6_0_8/Linux64bit+3.10-2.17/ups/setups .
  ln -s ups/v6_0_8/Linux64bit+3.10-2.17/ups/setup .
  popd
}


create_products_area $outdir

ups active|tail -n +2 | awk '{ printf "\"%s %s %s\"\n", $1, $2, $NF}'|while read i; do
    i=$(echo $i|tr '"' ' ')
    prd=$(echo $i|cut -d ' ' -f 1)
    prd_ver=$(echo $i|cut -d ' ' -f 2)
    products_dir=$(echo $i|cut -d ' ' -f 3)

    exclude_list="clang ups"
    #exclude_list="gcc boost clang hdf5 ups"
    if [[ $exclude_list =~ (^|[[:space:]])"$prd"($|[[:space:]]) ]]; then
        continue
    fi
    srcs=($products_dir/$prd/$prd_ver
	    $products_dir/$prd/${prd_ver}.version
	    $products_dir/$prd/current.chain
    )
    link_dests=($outdir/$prd/$prd_ver
	    $outdir/$prd/${prd_ver}.version
	    $outdir/$prd/current.chain
    )
    if [ -d ${srcs[0]} ]; then
	if [ -d ${link_dests[0]} ]; then
		echo "Info: ${link_dests[0]} exists, skip."
	else
		[[ -d `dirname ${link_dests[0]}` ]] && rm -rf `dirname ${link_dests[0]}`/*
		mkdir -p `dirname ${link_dests[0]}`
		for k in `seq 0 2`; do
                    if [ -d ${srcs[k]} ]; then
			echo "found ${srcs[k]}"
                        ln -s ${srcs[k]} ${link_dests[k]}
		    fi
                done
	fi
    fi
done 
