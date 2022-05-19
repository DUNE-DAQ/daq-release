#!/bin/bash

## Note: this script requiers pip2pi 
## It can be found at: https://github.com/dingp/pip2pi

manifest_file=$1
repo_path=$2
HERE=$(cd $(dirname $(readlink -f ${BASH_SOURCE})) && pwd)

$HERE/spack/make-release-repo.py -o /tmp -i $HERE/../configs/dunedaq-develop/dunedaq-develop.yaml --pypi-manifest
source /tmp/pypi_manifest.sh

# convert manifest YAML file to bash array?
source $manifest_file

function get_python_module {
  prd_list_name=$1[@]
  prd_list=("${!prd_list_name}")
  rel_prd_path=$2
  for prod in "${prd_list[@]}"; do
    iprd_arr=(${prod})
    prod_name=${iprd_arr[0]}
    prod_version=${iprd_arr[1]}
    prod_path=${iprd_arr[2]}
    echo "module: ${prod_name}, ${prod_version}, ${prod_path}"
    if [ $prod_path = "pypi" ]; then
	pip2pi $repo_path ${prod_name}==${prod_version}
    else
	github_user=${prod_path#*_}
	github_url="https://github.com/${github_user}/${prod_name}/archive/refs/tags/v${prod_version}.tar.gz"
        if [ "$prod_name" = "elisa-client-api" ]; then
            orig_prod_name="elisa_client_api"
	    github_url="https://github.com/${github_user}/${orig_prod_name}/archive/refs/tags/v${prod_version}.tar.gz"
        fi
	echo "Checking $github_url ..."
	if wget -q --method=HEAD $github_url; then
	    echo "Found tag v${prod_version} in GitHub repo ${github_user}/${prod_name}."
	else
	    echo "Tag v${prod_version} does not exist in GitHub repo ${github_user}/${prod_name}, trying the tag ${prod_version} without 'v' now."
	    github_url="https://github.com/${github_user}/${prod_name}/archive/refs/tags/${prod_version}.tar.gz"
	fi
	wget -O $repo_path/${prod_name}-${prod_version}.tar.gz $github_url 
    fi
  done
}

get_python_module dune_pythonmodules
dir2pi $repo_path
