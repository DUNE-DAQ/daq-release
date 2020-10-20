#!/bin/sh 

#set -x

ups list -Ksetups_dir:db:prod_dir_prefix | 

    sed -e 's;/[a-z]*/\.\./;/;g' | 
    grep "$SETUPS_DIR" | 
    ( 

       read setups_dir db prod_dir_prefix
       eval set :  $setups_dir $db $prod_dir_prefix
       setups_dir="$2"
       db="$3"
       prod_dir_prefix="$4"

       if [ "$setups_dir" = "$db" -a "$db" = "$prod_dir_prefix" ]
       then
	   cat > setups_layout <<EOF 
s_setenv UPS_THIS_DB \$SETUPS_DIR
s_setenv PROD_DIR_PREFIX \$SETUPS_DIR
EOF
	   exit
       fi

       base=`dirname $SETUPS_DIR`
       dbbase=`dirname $db`
       pdpbase=`dirname $prod_dir_prefix`
       base2=`dirname $base`

       case "$db" in
       $base/*)  echo s_setenv UPS_THIS_DB \$SETUPS_DIR/../`basename $db`;;
       $base2/*) echo s_setenv UPS_THIS_DB \$SETUPS_DIR/../../`basename $dbbase`/`basename $db`;;
       *)       echo s_setenv UPS_THIS_DB $db
       esac > ./setups_layout

       case "$prod_dir_prefix" in
       $base/*)  echo s_setenv PROD_DIR_PREFIX \$SETUPS_DIR/../`basename $prod_dir_prefix`;;
       $base2/*) echo s_setenv PROD_DIR_PREFIX \$SETUPS_DIR/../../`basename $pdpbase`/`basename $prod_dir_prefix`;;
       *)       echo s_setenv PROD_DIR_PREFIX $prod_dir_prefix
       esac >> ./setups_layout
  )
