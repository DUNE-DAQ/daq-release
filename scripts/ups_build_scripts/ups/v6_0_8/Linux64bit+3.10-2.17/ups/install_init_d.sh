#!/bin/sh

if [ "${UPS_VERBOSE}" = 1 ] 
then
    set -x
fi

for d in /etc/rc.d/init.d /sbin/init.d /etc/init.d
do
    if [ -d $d ]
    then
	sed -e 's;^upsdb=.*;'"upsdb=${UPS_THIS_DB};" < ${UPS_PROD_DIR}/init.d/ups > $d/ups
	chmod 755 $d
	chown root $d
	chgrp 0 $d
	$d/ups config 
    fi
done
