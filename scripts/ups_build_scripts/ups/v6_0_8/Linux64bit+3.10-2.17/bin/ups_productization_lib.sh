#! /bin/sh
# Source this file.
#  This file (ups_productization_lib.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
#  Jul  6, 2009. "TERMS AND CONDITIONS" governing this file are in the README
#  or COPYING file. If you do not have such a file, one can be obtained by
#  contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#  $RCSfile: ups_productization_lib.sh,v $
#  rev='$Revision: 1.25 $$Date: 2013/11/15 17:39:49 $'

test "`basename $0`" = ups_productization_lib.sh && echov() { if [ "${opt_v-}" ];then echo "`date`: $*";fi; }
echov "hello from ups_productization_lib.sh"

# NOTE: all "stages" (build,install,install_fix,and declare) are run in a
#       sub-shell. So do not set variables that are needed in other stages and
#       do not worry about preserving the cwd.

build()
{
    echov "build - PROD: $PROD_NAM $PROD_VER ${opt_qual-}"
    t0=`epoch`
    # look for a build function
    if type ${PROD_NAM}_build 2>/dev/null | grep 'is a function' >/dev/null;then
        echov prod specific build function found...
        ${PROD_NAM}_build "$@"
    else
        generic_build "$@"
    fi
    rsts=$?
    echov "build $* took `delta_m` minutes"
    return $rsts
}   # build
install()
{
    echov "install - PROD: $PROD_NAM $PROD_VER ${opt_qual-}"
    t0=`epoch`
    if type ${PROD_NAM}_install 2>/dev/null | grep 'is a function' >/dev/null;then
        echov prod specific install function found...
        ${PROD_NAM}_install "$@"
    else
        generic_install "$@"
    fi
    rsts=$?
    echov "install $* took `delta_m` minutes"
    return $rsts
}   # install
install_fix()
{
    echov "install_fix - PROD: $PROD_NAM $PROD_VER ${opt_qual-}"
    t0=`epoch`
    if type ${PROD_NAM}_install_fix 2>/dev/null | grep 'is a function' >/dev/null;then
        echov prod specific install_fix function found...
        ${PROD_NAM}_install_fix "$@"
    else
        generic_install_fix "$@"
    fi
    rsts=$?
    echov "install_fix $* took `delta_m` minutes"
    return $rsts
}   # install_fix
declare()
{
    echov "delare - PROD: $PROD_NAM $PROD_VER ${opt_qual-}"
    t0=`epoch`
    if type ${PROD_NAM}_declare 2>/dev/null | grep 'is a function' >/dev/null;then
        echov prod specific declare function found...
        ${PROD_NAM}_declare "$@"
    else
        generic_declare "$@"
    fi
    rsts=$?
    echov "declare $* took `delta_m` minutes"
    return $rsts
}   # declare

#------------------------------------------------------------------------------
# For the "generic" case, of the 4 stages (build,install,install_fix,and
# declare), I will dispatch to 2 of them -- the install_fix and declare stages
# will be the same for all.

generic_build()
{
    echov "generic build $*; PREFIX=$PREFIX"
    q_bld_dir=build-`basename $PREFIX`
    if [ "${1-}" = --status ];then
        if [ -d $q_bld_dir ];then return 0
        else                           return 1;fi
    else
        setup_deps
        generic_build_install_dispatch build
    fi
}   # generic_build
generic_install()
{
    echov "generic install $*; PREFIX=$PREFIX"
    if [ "${1-}" = --status ];then
        if [ -d $PREFIX ];then return 0
        else                       return 1;fi
    else
        generic_build_install_dispatch install
    fi
}   # generic_install
generic_install_fix()
{
    echov "generic install $*; PREFIX=$PREFIX"
    if [ "${1-}" = --status ];then
        # I want this a separate step, but have not determine a check
        return 1
    else
        if [ -d $PREFIX ];then   # the result of the install
            cd $PREFIX
            inst_PREFIX_fix $PREFIX
        else
            echo 'Error - generic install error'
            exit 1
        fi
    fi
}   # generic_install_fix
generic_declare()
{
    echov "generic declare $*; PREFIX=$PREFIX"
    if [ "${1-}" = --status ];then
        if [ -f $PRODS_RT/$PROD_NAM/$PROD_VER.version ];then return 0
        else                                                 return 1;fi
    else
        if [ -d $PREFIX ];then
            # only declare if this generic (qualified) inst dir exists
            if [ ! -f $PRODS_RT/$PROD_NAM/$PROD_VER/ups/$PROD_NAM.table \
                -o "${opt_redo-}" ];then
                echo mkdir -p $PRODS_RT/$PROD_NAM/$PROD_VER/ups
                mkdir -p $PRODS_RT/$PROD_NAM/$PROD_VER/ups
                add_fq --tblf=$UPS_DIR/ups/generic.table --flv=${PROD_FLV} --qual=${opt_qual-} \
                   >$PRODS_RT/$PROD_NAM/$PROD_VER/ups/$PROD_NAM.table
                #cp_with_deps $UPS_DIR/ups/generic.table \
                #    $PRODS_RT/$PROD_NAM/$PROD_VER/ups/$PROD_NAM.table
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

#------------------------------------------------------------------------------

ups_build()
{
    echov "ups build $*"
    if [ "${1-}" = --status ];then
        ups_flv=`get_flv_64_to_32`
        if [ -d build-$ups_flv ];then return 0
        else                          return 1;fi
    else

        # if Linux64bit -- try stripping 64bit and using CFLAGS=-m32
        if ups_flv=`get_flv_64_to_32`;then cflags="-cflags -m32"; fi

        if [ ! -d build-$ups_flv ];then mkdir build-$ups_flv;fi
        
        cd build-$ups_flv
        ups_dir=`pwd`
        ln -s ../inc .
        sts=0
        for dd in bin lib src man doc ups;do
            mkdir $dd
            cd $dd
            for ff in ../../$dd/*;do ln -s $ff .;done

            if [ -f Makefile ];then
                # MAKEFILE ADJUST SHOULD GO INTO CVS!!!
                if grep 'funame.*(OLD_LIBS)$' Makefile >/dev/null;then
                # fix for SunOS - tack  " $(LDFLAGS)" on the end of the funame
                #  compile command.  This is only really needed in the src
                #  directory where LDFLAGS is defined in the Makefile
                    sed '/funame.*OLD_LIBS/s/$/ $(LDFLAGS)/' Makefile \
                        >Makefile.$$
                    /bin/mv -f Makefile.$$ Makefile
                fi
                if [ "${opt_clean-}" ];then
                    UPS_DIR=$ups_dir ../../bin/upspremake clean
                fi
                PRODUCTS=$PRODS_RT UPS_DIR=$ups_dir ../../bin/upspremake -debug ${cflags-}
                _sts_=$?
                if [ $_sts_ -ne 0 ];then sts=$_sts_;fi
            fi

            cd ..
        done
        /bin/rm -fr src inc        # remove these "lndirs"

        return $sts
    fi
}   # ups_build

ups_install()
{
    echov "ups install $*"
    if [ "${1-}" = --status ];then
        if [ -d $PRODS_RT/$PROD_NAM/$PROD_VER/$PROD_FLV ];then return 0
        else                                                   return 1;fi
    else
        # b/c I'm not using 
        ups_flv=`get_flv_64_to_32`
        cd build-$ups_flv
        #inst_flv=`ups_flavor.sh --ups | sed 's/flavor=//;s/ .*//'` #ups_flavor.sh isn't working on SunOS
        inst_flv=$ups_flv
        inst_=$PRODS_RT/$PROD_NAM/$PROD_VER/$inst_flv
        mkdir -p $inst_
        /bin/cp -Lr bin lib man doc ups  $inst_
        unset inst_
    fi
}   # ups_install

ups_install_fix()
{
    echov "ups install_fix $*"
    if [ "${1-}" = --status ];then
        # even though nothing is done in the ups_install_fix,
        return 1 # so that the install will not be considered completed
    else
        :  nothing needs to be fixed
    fi
}   # ups_install_fix

ups_init_db() # $1=$PRODS_RT
{   mkdir -p $1/.upsfiles
    if [ -f $1/.upsfiles/dbconfig ];then
        echov $1/.upsfiles/dbconfig already exists
    else
            cat >$1/.upsfiles/dbconfig <<"EOF"
FILE = DBCONFIG
AUTHORIZED_NODES = *
PROD_DIR_PREFIX = ${UPS_THIS_DB}
#MAN_TARGET_DIR = /usr/local/man
#CATMAN_TARGET_DIR = /usr/local/catman
#SETUPS_DIR = /usr/local/etc
UPD_USERCODE_DIR = ${UPS_THIS_DB}/.updfiles
EOF
    fi
}

ups_declare()
{
    ups_flv=`get_flv_64_to_32`
    echov "ups declare $* ups_flv=$ups_flv"
    if [ "${1-}" = --status ];then
        # Can't tell which flavor would be built, so can't tell if its been declared
        return 1
    else

        # ups is special -- see if db is initializaed
        ups_init_db $PRODS_RT

        cd build-$ups_flv
        # ups_flavor not ready on SunOS inst_flv=`ups_flavor.sh --ups | sed 's/flavor=//;s/ .*//'`
        inst_flv=$ups_flv

# Should I act differently if under $PRODS_RT ????

        bin/ups declare -c -z$PRODS_RT -r$PROD_NAM/$PROD_VER/$inst_flv -Mups \
            -m$PROD_NAM.table -f$inst_flv $PROD_NAM $PROD_VER

        # now make sure the famous "setup" file is copied to $PRODS_RT
        if [ -f $PRODS_RT/setup -o -L $PRODS_RT/setup ];then
            if diff ups/setup $PRODS_RT >/dev/null;then
                echo "setup bootstrap file already in $PRODS_RT; copy skipped"
            else
                mv -f $PRODS_RT/setup $PRODS_RT.bak
                cp -f ups/setup $PRODS_RT
            fi
        else
            cp -f ups/setup $PRODS_RT
        fi
    fi
}   # ups_declare

#------------------------------------------------------------------------------


generic_configure_build()
{
    echov "generic \"./configure\" build (opt_configure=${opt_configure-})"
    mkdir -p $q_bld_dir
    cd $q_bld_dir
    if [ -f config.log ];then
        echo config.log exists
    fi
    flags=`qual_to_flags`
    # eval for flags and $PREFIX in $opt_configure
    cmd "$flags ../configure --prefix=$PREFIX ${opt_configure-}"
    qual_unO_Makefile
    test -f /proc/cpuinfo && j_opt=-j`grep processor /proc/cpuinfo | wc -l`
    make ${j_opt-} ${opt_make-}
}   # generic_configure_build
generic_configure_install()
{
    q_bld_dir=build-`basename $PREFIX`
    if [ -d $q_bld_dir ];then   # created by build stage
        cd $q_bld_dir
        if [ -f Makefile ];then
            prefix=`sed -n '/^ *prefix *=/{s/^[^=]*=[ 	]*//;p;q}' Makefile`
            if [ "$prefix" -a "$prefix" != $PREFIX ];then
                echo "Error - build/install mismatch"
                echo "   >$prefix<"
                echo "!= >$PREFIX<"
                exit 1
            fi
        else
            echo 'Error - no Makefile found. Can not make install'
            exit 1
        fi
        make install
    else
        echo 'Error - generic (build) install error'
        exit 1
    fi
}   # generic_configure_install

generic_cmake_build()
{   echov 'generic_cmake_build'
    mkdir -p $q_bld_dir
    cd $q_bld_dir
    flags=`qual_to_flags`
    cmd "$flags cmake .. -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX"
    qual_unO_Makefile
    make
}   # generic_cmake_build
generic_cmake_install()
{   echov 'generic_cmake_install'
    generic_configure_install
}   # generic_cmake_install

generic_bootstrap_build()
{   echov 'generic_bootstrap_build'
    eval "configure_opts=\"${opt_configure-}\""  # eval $PREFIX
    echov "generic \"./configure\" build (configure_opts=$configure_opts)"
    mkdir -p $q_bld_dir
    cd $q_bld_dir
    cmd "../bootstrap --prefix=$PREFIX"
    make
}   # generic_bootstrap_build
generic_bootstrap_install()
{   echov 'generic_bootstrap_install'
    generic_configure_install
}   # generic_bootstrap_install

generic_setup_py_build()
{   echov 'generic_setup_py_build'
    mkdir -p $q_bld_dir
    cd $q_bld_dir
    flags=`qual_to_flags`
    cmd "$flags python ../setup.py build"
}   # generic_setup_py_build
generic_setup_py_install()
{   echov 'generic_setup_py_install'
    q_bld_dir=build-`basename $PREFIX`
    if [ -d $q_bld_dir ];then   # created by build stage
        cd $q_bld_dir
        cmd "python ../setup.py install --prefix=$PREFIX"
    else
        echo 'Error - generic (build) install error'
        exit 1
    fi
}   # generic_setup_py_install

generic_Makefile_build()
{   echov 'generic_Makefile_build'
}   # generic_Makefile_build
generic_Makefile_install()
{   echov 'generic_Makefile_install'
}   # generic_Makefile_install

#------------------------------------------------------------------------------

calc() { echo "scale=4;$*" | bc; }
#epoch() { date +%s; }
epoch() { perl -e 'use Time::Local;print timelocal(localtime());'; }
delta_m() { t1=`epoch`; delta_s=`expr $t1 - $t0`; calc "$delta_s / 60"; }

qual_to_flags()
{
    if echo :${opt_qual-}: | grep :debug: >/dev/null;then
        echo "CFLAGS=\"$CFLAGS -g -O0\" CXXFLAGS=\"$CXXFLAGS -g -O0\""
    elif echo :${opt_qual-}: | grep :cxxcheck: >/dev/null;then
        echo "CFLAGS=\"$CFLAGS -g -O0\" CXXFLAGS=\"$CXXFLAGS -g -O0\"\
 CPPFLAGS=\"$CPPFLAGS -D_GLIBCXX_DEBUG\""
    fi
}

qual_unO_Makefile()
{
    if echo :${opt_qual-}: | egrep ':(debug|cxxcheck):' >/dev/null;then
        if [ ! -f Makefile.orig ];then cp -p Makefile Makefile.orig;fi
        sed -i -e '/[ "]-O[s1-3 "]/s/-O[s1-3 ]//' \
               -e '/[ "]-O$/s/-O//'        Makefile  # " quote for emacs
                                                     #  colorization
    fi
}

inst_PREFIX_fix()
{
    re="^#! *$PREFIX/bin/"
    ff_l=`find . -type f | xargs grep -l "$re"`
    for ff in $ff_l;do
        if grep "${re}[^ ]*  *-" $ff >/dev/null;then
            echo 'Trouble -- interp with option(s)'
        else
            echov "fixing $ff"
            # what about permissions???
            sed -i "s|$re|#!/usr/bin/env |" $ff
        fi
    done
    re="$PREFIX"
    ff_l=`find . -type f | xargs grep -l "$re"`
    for ff in $ff_l;do
        ere='script|ELF|English|ASCII text|archive'
        file_t=`file $ff | egrep -o "$ere" | sed 's/ //'`
        ff_basename=`basename $ff`
        ext=`expr "$ff_basename" : '.*\.\([^.]*\)'`
        echov "$ff: >$ext $file_t<"
        case "$ext $file_t" in
        'rb English')  # ruby file
            # let flavor[_qual] stay
            sed -i "s|$PRODS_RT/$PROD_NAM/$PROD_VER|#{ENV[\"RUBY_DIR\"]}|" $ff
            ;;
        esac
    done
}

echo_BLD_INST_METHOD()
{
    if [ "${BLD_INST_METHOD-}" = '' ];then
        if   [ -f bootstrap ];then
            BLD_INST_METHOD=bootstrap
        elif [ -f configure ];then
            BLD_INST_METHOD=configure
        elif xx=`/bin/ls *.cmake 2>/dev/null`;then
            if [ `echo "$xx"|wc -l` -gt 1 ];then
                echo "Warning - more than 1 cmake" >&2
            fi
            BLD_INST_METHOD=cmake
        elif [ -f setup.py ];then
            BLD_INST_METHOD=setup_py
        elif [ -f Makefile ];then
            BLD_INST_METHOD=Makefile
        else
            return 1
        fi
    fi
    echo $BLD_INST_METHOD
}

generic_build_install_dispatch()
{
    # of course, the order of checking is important!!!
    if BLD_INST_METHOD=`echo_BLD_INST_METHOD`;then
        : OK
    else
        echo "Error - cannot determine \"generic\") method (for $1 stage)."
        echo 'You must create a <prod>_productization_lib.sh file'
        echo "for $PROD_NAM"
        exit 1
    fi
    case $1 in
    build|install)   generic_${BLD_INST_METHOD}_$1;;
    *)  echo 'Error - internal';exit 1;;
    esac
}

flavor_qualifier_dir()
{
    if [ "$PROD_QUALS" ];then
        # FIXME -- strip out "non-build qualifiers" (aka "environment qualifiers")
        if [ -x $UPS_DIR/ups/ups_FQ ];then
            qq=`$UPS_DIR/ups/ups_FQ --get '' $PROD_FLV $PROD_QUALS`
        else
            qq=${PROD_FLV}_`echo $PROD_QUALS | sed 's/:/_/g'`
        fi
    else
        qq=$PROD_FLV
    fi
    # check for bin dist
    Pwd=`pwd`
    if nam_ver=`expr "$Pwd" : "$PRODS_RT/\([^/][^/]*/[^/][^/]*\)\$"`;then
        echo 'flavor_qualifier_dir' >&2
        #for dd in `find . -type d -maxdepth 3`;do
        for dd in `find . -type d`;do
            if   expr "$dd" : '.*/bin$' >/dev/null;then
                if file $dd/* | grep 'ELF ' >/dev/null;then have_bin=$dd; fi
            elif expr "$dd" : '.*/lib$' >/dev/null;then
                if file $dd/* | grep 'ELF ' >/dev/null;then have_lib=$dd; fi
            fi
        done
        if [   \( "${have_bin-}" -a "${have_lib-}" = '' \) \
            -o \( "${have_lib-}" -a "${have_bin-}" = '' \) \
            -o \( "${have_lib-}" -a "${have_bin-}" = "${have_bin-}" \) ];then
            if [ "${have_bin-}" ];then bb="$have_bin"; else bb="$have_lib";fi
        fi
        if [ "${bb-}" -a "`echo_BLD_INST_METHOD`" = '' ];then
            echo 'looks like "binary dist."' >&2
            bb=`expr "$bb" : '\./\(.*\)/...$'`   # no match OK for ./bin
            # now bb may be '' (if bb was ./bin or ./lib)
            if [ "$bb" != $qq ];then
                echo "Warning $bb != $qq" >&2
            fi
            echo $bb
        else
            echo $qq
        fi
    else
        # source dir
        echo $qq
    fi
}

setup_deps()
{
    if [ "${opt_deps-}" ];then
        IFSsav=$IFS IFS=,; for dep in $opt_deps;do IFS=$IFSsav
            setup $dep
            if [ $? -ne 0 ];then
                echo "Error - error setting up $dep"
                exit 1
            fi
        done
    fi
}

cp_with_deps()
{
    tblf_i=$1
    tblf_o=$2
    if [ "${opt_deps-}" ];then
        sed_up=''
        sed_dn=''
        IFSsav=$IFS IFS=,; for dep in $opt_deps;do IFS=$IFSsav
            dep=`echo $dep`
            echov "dep=>$dep<"
            sed_up="$sed_up\\        setupRequired( \"$dep\" )\\n"
            # unsetup in opposite order
            sed_dn="\\        unsetupRequired( \"$dep\" )\\n$sed_dn"
        done
        if xx=`grep -io '^[  ]*action[       ]*=[    ]unsetup' $tblf_i`;then
            : echo table file has unsetup
        else
            echo 'Error - missing unsetup in table file'
            exit 1
        fi

        sed "\
/$xx/i$sed_up\n
/$xx/a$sed_dn
" $tblf_i >$tblf_o
    else
        cp $tblf_i $tblf_o
    fi
}   # cp_with_deps
cp_with_vars()
{
    tblf_i=$11; shift
    tblf_o=$1; shift
    if [ $# -gt 0 ];then
        sed_up=''
        sed_dn=''
        set -- "$@" ${opt_var-}
        while var=`expr "${1-}" : '\([^=]*\)'`;do
            val=`expr "$1" : '[^=]*=\(.*\)'`
            shift
            sed_up="$sed_up\\        envSet( $var, $val )\\n"
            # unsetup in opposite order
            sed_dn="\\        envUnset( $var )\\n$sed_dn"
        done
        if xx=`grep -io '^[  ]*action[       ]*=[    ]unsetup' $tblf_i`;then
            : echo table file has unsetup
        else
            echo 'Error - missing unsetup in table file'
            exit 1
        fi

        sed "\
/$xx/i$sed_up\n
/$xx/a$sed_dn
" $tblf_i >$tblf_o
    else
        cp $tblf_i $tblf_o
    fi
}   # cp_with_vars


add_fq()
{
    set_fq_opt "$@"
    set_TBLF
    set_FLVR
    echo "add_fq aft set_FLVR FLVR=${FLVR-}" >&2
    set_QUAL
    echo "add_fq aft set_QUAL" >&2
    if [ "${opt_deps-}" ];then
        sed_up=''
        IFSsav=$IFS IFS=,; for dep in $opt_deps;do IFS=$IFSsav
            dep=`echo $dep`
            echo "dep=>$dep<"
            sed_up="$sed_up\\        setupRequired( \"$dep\" )\\n"
        done
    fi
    echo "add_fq b4 list_fq" >&2
    if list_fq | grep "$FLVR \"$QUAL\"";then
        echo "flv/qual combination already exists"
    else
        sed "\
/^GROUP:/a\
\\\n\
\\FLAVOR = $FLVR\nQUALIFIERS = \"$QUAL\"\n\
\\    ACTION = setup\n${sed_up-}\
\\        exeActionRequired(\"setup__\")\n\
" $TBLF
    fi
    echo "add_fq finish" >&2
}
list_fq()
{
    set_fq_opt "$@"
    set_TBLF
    sed -n -e'/^FLAVOR *=/{s/^FLAVOR *= *//;h;}'\
           -e'/^QUALIFIERS *=/{s/^QUALIFIERS *= */ /;H;x;s/\n//;p}' $TBLF
}

#------------------------------------------------------------------------------
set_TBLF()
{   if [ "${TBLF-}" ];then return; fi
    if [ "${opt_tblf-}" ];then TBLF=$opt_tblf
    else                       TBLF=$UPS_DIR/ups/generic.table; fi
}
set_FLVR()
{   echo "set_FLV called" >&2
    if [ "${FLVR-}" ];then return; fi
    if [ "${opt_flv-}" ];then FLVR=$opt_flv
    else                      FLVR=`ups flavor`; fi
    echo "setting FLVR=$FLVR" >&2
}
set_QUAL()
{   if [ "${QUAL-}" ];then return; fi
    if [ "${opt_qual-}" ];then QUAL=$opt_qual
    else                       QUAL=; fi
}
set_fq_opt()
{
    opts_w_args='tblf|flv|qual|deps'
    do_opt="\
    case \$opt in
    $opts_w_args)  if   [ \"\${rest-}\" != '' ];then arg=\$rest; rest=
                   elif [      $#      -ge 1  ];then arg=\$1; shift
                   else  echo \"option \$opt requires argument\"; exit 1; fi
                   eval opt_\`echo \$opt|sed -e 's/-/_/g'\`=\"'\$arg'\"
                   opts=\"\$opts '\$arg'\";;      # tricky part Aa
    *) echo \"internal fq function error - valid option: \$opt\" 2>&1; exit 1;;
    esac"
    while op=`expr "${1-}" : '-\(.*\)'`;do
        shift
        if xx=`expr "$op" : '-\(.*\)'`;then
            if   opt=`expr     "$xx" : '\([^=]*\)='`;then
                set ${-+-$-} -- "`expr "$xx" : '[^=]*=\(.*\)'`" "$@"
            else opt=$xx; fi
            opts="${opts-} '--$opt'"      # tricky part Ab
            eval "$do_opt"
        else
            while opt=`expr "$op" : '\(.\)'`;do
                opts="${opts-} '-$opt'"      # tricky part Ac
                rest=`expr "$op" : '.\(.*\)'`
                eval "$do_opt"
                op=$rest
            done
        fi
    done
}   # set_fq_opt

flavor()
{
    os=`uname -s` os_rev=`uname -r | cut -d. -f1-2` mach=`uname -m` libc=
    if [ $os = Linux ];then
        libc=-`/bin/ls /lib/libc-* | sed '{s/^[^0-9]*//;s/[^0-9]*$//;q}'`
    fi
    case $mach in
    x86_64|sun*) b64=64bit;;
    *)           b64=;;
    esac
    fl1=$os$b64+$os_rev$libc
    fl2=$os+$os_rev$libc
    fl3=$os+`expr "$os_rev" : '\([0-9]*\)'`
    case "${1-}" in
    ''|-4)      echo $fl1;;
    -[1-3])  nn=`expr "$1" : '-\(.\)'`;expr "$fl1" : "\(\([^-+.]*[-+.]\)\{$nn\}\)" | sed 's/.$//';;
    *)       echo $fl3;;
    esac
}

legacy_flavor()
{
    flv_=$1
    case $flv_ in
    Linux+2.4-2.3.2)        echo Linux;;
    Linux+2.6-2.3.4)        echo Linux+2.6;;
    Linux+2.6-2.5)          echo Linux+2.6-2.5;;
    Linux64bit+2.4-2.3.2)   echo Linux64bit;;
    Linux64bit+2.6-2.3.4)   echo Linux64bit+2.6;;
    Linux64bit+2.6-2.5)     echo Linux64bit+2.6-2.5;;
    *)                      echo $flv_;;
    esac
}

set_FLV()
{
    PROD_FLV=`flavor`
}

get_flv_64_to_32()
{
    legacy_flavor `echo $PROD_FLV | sed 's/64bit//'`
    if expr "$PROD_FLV" : '.*64bit' >/dev/null;then return 0
    else                                            return 1; fi
}

lndir()
{   src=$1
    dst=$2
    cdsav=`pwd`
    cd $dst
    abs_dst=`pwd`
    abs_src=`cd $src;pwd`
    abs_chk_len=`expr "$abs_dst/" : '.*'`
    src_slash_cnt=`echo $src | sed 's/[^/]//g' | wc -c`
    src_slash_cnt=`expr $src_slash_cnt - 1`
    (cd $src;find . -type d ) | \
    while read dd;do
        dd=`expr "$dd" : '\.\(.*\)'` # strip off leading "."
        if [ `expr "$abs_src$dd/" : "$abs_dst/"` -eq $abs_chk_len ];then
            echo skipping $abs_src$dd >&2
            continue
        fi
        if [ "$dd" ];then  mkdir .$dd; fi

        if expr "$src" : / >/dev/null;then
            extra_up=
        else
            slash_cnt=`echo $dd | sed 's/[^/]//g' | wc -c`
            extra_up=
            slash_cnt=`expr $slash_cnt - $src_slash_cnt`
            if [ $slash_cnt -ge 1 ];then
                while slash_cnt=`expr $slash_cnt - 1`;do
                    extra_up="../$extra_up"
                done
            fi
        fi

        no_prune=`basename "$src$dd"`
        find "$src$dd" \! -type d -o -type d \! -name "$no_prune" -prune \! -type d | \
        while read ff;do ln -s $extra_up$ff .$dd; done
    done
    cd $cdsav
    unset cdsav src dst abs_src abs_dst abs_chk_len src_slash_cnt
}   # lndir

#------------------------------------------------------------------------------
# allow for testing of some functions...
test "x${1-}" = x-v && opt_v=1 && shift
case "${1-}" in
'-?'|-h|--help) echo "usage: `basename $0` flavor|cp_with_vars|init_db";;
flavor)  shift; flavor "$@";;
cp_with_vars) shift; cp_with_vars 'ROOTSYS=${UPS_PROD_DIR}' "$@";;
init_db) shift; if [ $# -eq 1 ];then ups_init_db $1;else echo 'init_db needs 1 arg (prod/db root)';exit 1;fi;;
esac
