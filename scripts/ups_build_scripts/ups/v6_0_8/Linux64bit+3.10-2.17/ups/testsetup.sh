#!/bin/sh
set -u   # this is the point of this test script
USAGE="\
   usage: `basename $0` <prod_db_path>
examples: `basename $0` \$HOME/p
          sh -x $0 \$HOME/p
"
if [ $# -ne 1 ];then echo "$USAGE"; exit; fi
if [ ! -f $1/setup ];then echo "$1/setup file does not seem to exist"; exit; fi

vv=1

echo "First, without prod_db set:"
sleep 1
. $1/setup

sleep 2
echo "Now with prod_db set:"
sleep 1
prod_db=$1
. $prod_db/setup

sleep 2
echo "test complete"
