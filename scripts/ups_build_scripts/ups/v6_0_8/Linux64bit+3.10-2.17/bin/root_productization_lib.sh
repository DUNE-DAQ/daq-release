#! /bin/sh
#  This file (root_productization_lib.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
#  Jul 14, 2009. "TERMS AND CONDITIONS" governing this file are in the README
#  or COPYING file. If you do not have such a file, one can be obtained by
#  contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#  $RCSfile: root_productization_lib.sh,v $
#  rev='$Revision: 1.4 $$Date: 2009/12/07 21:27:26 $'


# root does not support out of source builds.
# The lndir trick does not work (completely).
# With lndir, the make install ends up installing
# over 2000 symlinks from the build dir.


generic_build()
{
    echov "root_productization build $@; PREFIX=$PREFIX"
    q_bld_dir=build-`basename $PREFIX`
    export ROOTSYS; ROOTSYS=$PREFIX
    if [ "${1-}" = --status ];then
        if [ -d $q_bld_dir ];then return 0
        else                      return 1;fi
    else
        setup_deps
        mkdir -p $q_bld_dir
        cd $q_bld_dir
        cmd lndir .. .
        flags=`qual_to_flags`
        # eval for flags and $PREFIX in $opt_configure
        cmd "$flags ./configure ${opt_configure-}"
        qual_unO_Makefile
        test -f /proc/cpuinfo && j_opt=-j`grep processor /proc/cpuinfo | wc -l`
        make ${j_opt-} ${opt_make-}
    fi
}
generic_install_fix()
{
    echov "root_productization install_fix $@; PREFIX=$PREFIX"
    if [ "${1-}" = --status ];then
        # I want this a separate step, but have not determine a check
        return 1
    else
        if [ -d $PREFIX ];then   # the result of the install
            cd $PREFIX
            find . -type l | while read ll;do rm $ll; cp $DIST_DIR/$ll $ll;done
        else
            echo 'Error - generic install error'
            exit 1
        fi
    fi
}   # generic_install_fix
generic_declare()
{
    echov "root_productization declare $@; PREFIX=$PREFIX"
    if [ "${1-}" = --status ];then
        # NOTE: this portion not in sub-shell -- setting env.vars here will
        #       effect future sub-shells (but watchout for --stages option)
        if [ -f $PRODS_RT/$PROD_NAM/$PROD_VER.version ];then return 0
        else                                                 return 1;fi
    else
        if [ -d $PREFIX ];then
            # only declare if this generic (qualified) inst dir exists
            if [ ! -f $PRODS_RT/$PROD_NAM/$PROD_VER/ups/$PROD_NAM.table \
                -o "${opt_redo-}" ];then
                echo mkdir -p $PRODS_RT/$PROD_NAM/$PROD_VER/ups
                mkdir -p $PRODS_RT/$PROD_NAM/$PROD_VER/ups
                ups_table.sh add_fq --envvar='ROOTSYS=\${UPS_PROD_DIR}/\$\${UPS_PROD_NAME_UC}_FQ'\
                    -f ${PROD_FLV} ${opt_qual-}\
                   >$PRODS_RT/$PROD_NAM/$PROD_VER/ups/$PROD_NAM.table
            fi
        
            cmd "ups declare -c -z$PRODS_RT -r$PROD_NAM/$PROD_VER -Mups \
                -m$PROD_NAM.table -f$PROD_FLV ${opt_qual+-q$opt_qual} \
                $PROD_NAM $PROD_VER"
        else
            echo 'Error - generic (qualified) inst dir does not exists'
            exit 1
        fi
    fi
}   # generic_declare
