#! /bin/sh
#  This file (root_productization_lib.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
#  Jul 14, 2009. "TERMS AND CONDITIONS" governing this file are in the README
#  or COPYING file. If you do not have such a file, one can be obtained by
#  contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#  $RCSfile: boost_productization_lib.sh,v $
#  rev='$Revision: 1.1 $$Date: 2009/07/15 04:25:49 $'


generic_build()
{
    echov "boost build $@; PREFIX=$PREFIX"
    q_bld_dir=build-`basename $PREFIX`
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
        cmd "$flags ./configure --prefix=$PREFIX ${opt_configure-}"
        qual_unO_Makefile
        make
    fi
}
