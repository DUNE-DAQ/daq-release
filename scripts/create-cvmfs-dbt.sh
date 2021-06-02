#!/bin/bash

TARGET_DIR=/home/dingpf/cvmfs_dune/dunedaq/DUNE/tools/
##TARGET_DIR=/home/dingpf/cvmfs_dunedaq/tools/

mkdir -p /var/tmp
cd /var/tmp
rm -rf dbt
rm -rf daq-buildtools
git clone git@github.com:DUNE-DAQ/daq-buildtools.git

cd daq-buildtools

mkdir -p /var/tmp/dbt
for i in `git tag`; do 
	if [[ $i = v* ]]; then
		git archive --format=tar --prefix=$i/ $i | (cd /var/tmp/dbt && tar xf -)
	fi
done

for i in `git tag`; do 
	if [[ $i = v* ]]; then
		for j in `git tag --points-at $(git show-ref -s $i)`; do
			if [[ $j = "dunedaq-v"* ]]; then
				cd /var/tmp/dbt
				echo $i $j
				ln -s $i $j
				cd /var/tmp/daq-buildtools
			fi
		done
	fi
done

rsync -vau /var/tmp/dbt $TARGET_DIR

rm -rf /var/tmp/dbt
rm -rf /var/tmp/daq-buildtools
