
. unittest.bash

setup_test_db() {

   SAVEPRODUCTS=$PRODUCTS
   export workdir=/tmp/p$$
   . `ups setup -j upd v4_8_0`
   mkprd -r $workdir
   echo 'SETUPS_DIR=${UPS_THIS_DB}' >> $workdir/.upsfiles/dbconfig
   PRODUCTS=$workdir
   SETUPS_DIR=$workdir
   mkdir -p  $workdir/ups/devel
   cp -r $UPS_DIR $workdir/ups/devel/Linux+2
   ups declare -r $workdir/ups/devel/Linux+2 -C -M ups -m ups.table ups devel -2 
}

teardown_test_db() {
   rm -rf $workdir
   PRODUCTS=$SAVEPRODUCTS
}

test_env() {
   echo PRODUCTS is $PRODUCTS
   echo UPS_DIR is $UPS_DIR
   ups list -aK+:prod_dir:ups_dir:declared:declarer ups 
   [ `ups list -aK+ ups | wc -l` = 1 ]
}

test_current() {
   rm $SETUPS_DIR/setups_layout
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   ls -al $workdir
   ups list -a 

   ls $SETUPS_DIR
   echo "workdir/set.." `ls -l $workdir/set* | wc -l` 
   echo "count ups" `ups list -aK+ ups | wc -l` 

   [ -r $SETUPS_DIR/setups_layout ] &&
   [ `ls -l $workdir/set* | wc -l` = 8 ] &&
   [ `ups list -aK+ ups | wc -l` = 1 ]
}

test_one_deep() {
   rm $SETUPS_DIR/setups_layout
   ups undeclare ups devel
   mv $PRODUCTS/ups/devel/Linux+2 $PRODUCTS/ups/foo
   rmdir $PRODUCTS/ups/devel
   mv $PRODUCTS/ups/foo $PRODUCTS/ups/devel
   ups declare -c -r $workdir/ups/devel -M ups -m ups.table ups devel -2 
   ls -al $workdir
   ups list -a 

   ls $SETUPS_DIR
   echo "workdir/set.." `ls -l $workdir/set* | wc -l` 
   echo "count ups" `ups list -aK+ ups | wc -l` 

   [ -r $SETUPS_DIR/setups_layout ] &&
   [ `ls -l $workdir/set* | wc -l` = 8 ] &&
   [ `ups list -aK+ ups | wc -l` = 1 ]
}

test_no_current_action_bash() {
   ups list -aK+ ups
   ups declare -C -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   cp $workdir/ups/devel/Linux+2/ups/setups $workdir/setups
   cat > $workdir/setups_layout <<EOF
s_setenv UPS_THIS_DB $SETUPS_DIR
s_setenv PROD_DIR_PREFIX $SETUPS_DIR
EOF
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/bash << EOF
       printenv
       set -x
       . $workdir/setups
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
EOF
}
test_no_current_action_zsh() {
   ups list -aK+ ups
   ups declare -C -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   cp $workdir/ups/devel/Linux+2/ups/setups $workdir/setups
   cat > $workdir/setups_layout <<EOF
s_setenv UPS_THIS_DB $SETUPS_DIR
s_setenv PROD_DIR_PREFIX $SETUPS_DIR
EOF
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/zsh << EOF
       printenv
       set -x
       . $workdir/setups
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
EOF
}

test_no_current_action_csh() {
   # note we have to run it with "script" so it looks interactive and
   # has $_ to test that it can find itself...

   ups declare -C -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   cp $workdir/ups/devel/Linux+2/ups/setups $workdir/setups
   cat > $workdir/setups_layout <<EOF
s_setenv UPS_THIS_DB $SETUPS_DIR
s_setenv PROD_DIR_PREFIX $SETUPS_DIR
EOF
   
   script -c  "env -i PATH=/bin:/usr/bin HOME=$HOME /bin/csh" -f << EOF
       printenv
       set echo
       source  $workdir/setups
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
       exit $?
EOF
}

test_empty_sh() {
   ups list -aK+ ups
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/sh << EOF
       printenv
       set -x
       . $workdir/setups.sh
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
EOF
}

test_empty_dash() {
   ups list -aK+ ups
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/dash << EOF
       printenv
       set -x
       . $workdir/setups.sh
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
EOF
}

test_empty_zsh() {
   ups list -aK+ ups
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/zsh << EOF
       printenv
       set -x
       . $workdir/setups.sh
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
EOF
}

test_empty_csh() {
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/csh -f << EOF
       printenv
       set echo
       source  $workdir/setups.csh
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
       exit $?
EOF
}

test_expect_csh() {
   export workdir
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   SOURCEIT=source TESTSHELL="csh -f" ./test_setup.exp | tee /tmp/out$$
   grep "$workdir.*/ups" /tmp/out$$
}

test_expect_bash() {
   export workdir
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   SOURCEIT=. TESTSHELL="bash --norc --noprofile --login " ./test_setup.exp | tee /tmp/out$$
   grep "$workdir.*/ups" /tmp/out$$
}

test_expect_zsh() {
   export workdir
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   SOURCEIT=. TESTSHELL=zsh ./test_setup.exp | tee /tmp/out$$
   grep "$workdir.*/ups" /tmp/out$$
}

test_expect_dash() {
   export workdir
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   SOURCEIT=. TESTSHELL="dash" ./test_setup.exp | tee /tmp/out$$
   grep "$workdir.*/ups" /tmp/out$$
}

test_scriptname_sh() {
   ups list -aK+ ups
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/sh << EOF
       printenv
       set -x
       sed -e 's/^[ 	]*//'  > $workdir/drink_me <<NEOF
       #!/bin/sh
       . $workdir/setups.sh
       ups list -aK+ ups
NEOF
       chmod +x $workdir/drink_me
       $workdir/drink_me
       [  \`$workdir/drink_me | wc -l\` = 1  ]
EOF
}

test_scriptname_dash() {
   ups list -aK+ ups
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/dash << EOF
       printenv
       set -x
       sed -e 's/^[ 	]*//'  > $workdir/drink_me <<NEOF
       #!/bin/sh
       . $workdir/setups.sh
       ups list -aK+ ups
NEOF
       chmod +x $workdir/drink_me
       $workdir/drink_me
       [  \`$workdir/drink_me | wc -l\` = 1  ]
EOF
}

test_scriptname_zsh() {
   ups list -aK+ ups
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/zsh << EOF
       printenv
       set -x
       sed -e 's/^[ 	]*//'  > $workdir/drink_me <<NEOF
       #!/bin/zsh
       . $workdir/setups.sh
       ups list -aK+ ups
NEOF
       chmod +x $workdir/drink_me
       $workdir/drink_me
       [  \`$workdir/drink_me | wc -l\` = 1  ]
EOF
}

test_scriptname_csh() {
   ups list -aK+ ups
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   /bin/bash -c "source $workdir/setups"

   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/csh -f << EOF
       printenv
       set echo
       sed -e 's/^[ 	]*//'  > $workdir/drink_me <<NEOF
       #!/bin/csh -f
       set echo
       source $workdir/setups.csh
       ups list -aK+ ups
NEOF
       chmod +x $workdir/drink_me
       $workdir/drink_me
       [  \`$workdir/drink_me | wc -l\` = 1  ]
       exit $?
EOF
}

test_over_old() {
   upd install -G -c ups v4_7
   # ups declare -c ups devel -2
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
    
   ls -l $workdir/set*
   ls -l $workdir/.old

   [ `ls -l $workdir/set* | wc -l` = 7 ] &&
   [ `ups list -K+ ups | wc -l` = 1 ]

}

test_triple_sh() {
   ups list -aK+ ups
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/sh << EOF
       printenv
       set -x
       . $workdir/setups.sh
       . $workdir/setups.sh
       . $workdir/setups.sh
       printenv
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
EOF
}

test_triple_csh() {
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/csh -f << EOF
       printenv
       set echo
       source  $workdir/setups.csh
       source  $workdir/setups.csh
       source  $workdir/setups.csh
       printenv
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
       exit $?
EOF
}

test_triple_zsh() {
   ups list -aK+ ups
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/zsh << EOF
       printenv
       set -x
       . $workdir/setups.sh
       . $workdir/setups.sh
       . $workdir/setups.sh
       printenv
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
EOF
}

test_quiet_sh() {
   ups list -aK+ ups
   ups declare -c ups devel -2 || true
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/sh > /tmp/out$$ 2>&1 << EOF
       . $workdir/setups.sh
       . $workdir/setups.sh
       . $workdir/setups.sh
EOF
   echo "output: "
   cat /tmp/out$$
   [ `wc -c < /tmp/out$$` = 0 ]
}

test_quiet_csh() {
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/csh -f > /tmp/out$$ 2>&1  << EOF
       source  $workdir/setups.csh
       source  $workdir/setups.csh
       source  $workdir/setups.csh
       exit $?
EOF
   echo "output: "
   cat /tmp/out$$
   [ `wc -c < /tmp/out$$` = 0 ]
}

test_local_PRODUCTS_sh() {
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/sh << EOF
       printenv
       set -x
       # NOTE: NOT exported...
       PRODUCTS=/some/place  
       . $workdir/setups.sh
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
EOF
}

test_local_PRODUCTS_csh() {
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/csh -f << EOF
       printenv
       set echo
       # NOTE: NOT setenv
       set PRODUCTS=/some/place
       source  $workdir/setups.csh
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` = 1  ]
       exit $?
EOF
}

test_afs_products_sh() {
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2 
   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/sh << EOF
       printenv
       set -x
       # NOTE: NOT exported...
       export PRODUCTS=/afs/fnal.gov/ups/db
       . $workdir/setups.sh
       ups list -aK+ ups
       [  \`ups list -aK+ ups | wc -l\` -gt 10  ]
EOF
}

#
# no longer used, since /afs/fnal.gov is gone now..
#
test_override_hostname_file() {
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2

   echo "-H `ups flavor -4`" > $workdir/.upsfiles/ups_OVERRIDE.`hostname`

   ls -l $workdir/.upsfiles

   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/sh << EOF
       printenv
       set -x
       . $workdir/setups.sh

       ups list -aK+ ups
       echo override is \$UPS_OVERRIDE
       
       [ "x\$UPS_OVERRIDE" != x  ]
EOF
}
test_override_kvers_file() {
   ups declare -c -r $workdir/ups/devel/Linux+2 -M ups -m ups.table ups devel -2   
   echo "-H `ups flavor -4`" > $workdir/.upsfiles/ups_OVERRIDE.`uname -r | cut -d. -f 1-2`
   ls -l $workdir/.upsfiles

   env -i PATH=/bin:/usr/bin HOME=$HOME /bin/sh << EOF
       printenv
       set -x
       . $workdir/setups.sh
       ups list -aK+ ups
       echo override is \$UPS_OVERRIDE
       env | grep OVERRIDE
       
       [ "x\$UPS_OVERRIDE" != x  ]
EOF
}

testsuite setups_suite \
        -s setup_test_db \
	-t teardown_test_db \
	test_env   	\
	test_current	\
        test_one_deep   \
        test_expect_csh \
        test_expect_bash \
        test_expect_zsh \
        test_expect_dash \
	test_no_current_action_bash	\
	test_no_current_action_csh  \
	test_no_current_action_zsh  \
	test_empty_sh	\
	test_empty_csh  \
	test_empty_zsh  \
	test_empty_dash  \
	test_scriptname_sh	\
	test_scriptname_csh  \
	test_scriptname_zsh  \
	test_scriptname_dash  \
        test_triple_sh  \
        test_triple_csh \
        test_triple_zsh \
        test_quiet_sh   \
        test_quiet_csh  \
        test_local_PRODUCTS_sh \
        test_local_PRODUCTS_csh \
        test_override_hostname_file \
        test_override_kvers_file 

setups_suite "$@"

