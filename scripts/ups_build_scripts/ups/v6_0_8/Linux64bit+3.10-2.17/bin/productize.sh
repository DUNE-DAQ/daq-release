#! /bin/sh
#  This file (ups-productize.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
#  Jul  4, 2009. "TERMS AND CONDITIONS" governing this file are in the README
#  or COPYING file. If you do not have such a file, one can be obtained by
#  contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#  $RCSfile: productize.sh,v $
#  rev='$Revision: 1.23 $$Date: 2011/01/28 17:11:31 $'
if [ "$1" = -x ];then set -x;shift;fi
/bin/sh -c 'set -u;x="$*"' 2>/dev/null && set -u

APP=`basename $0`
USAGE="\
   usage: $APP <path/to/src>
examples: $APP --prod=ups --ver=4.7.4x some/dir
          $APP --deps='prodA v1, prodB v2' some/dir
          $APP --deps='prodA v1 -qdebug, prodB v2 -qdebug' -qdebug some/dir
          cd some/dir
          $APP --deps='prodA vX, prodB vZ' -qcms .
          $APP --deps='prodA vX -qdebug, prodB vZ -qdebug' -qcms:debug .

If PWD is not in the form prod-ver, then --prod and --ver options must be
supplied.
Options:
--prod=
--ver=
--prods-root=          or 1st \"db\" in PRODUCTS env.var. (IF \"db\" is a
                        combined db/prod_root)
--deps=
--var=
-q | --quals=          understand: debug,cxxcheck; others just used in declare
--productization-lib=
--stages=              dflt: build:install:declare
--configure=           *B configure options (for prods that use \"configure\")
--clean                *B
#--no-src               *I do not copy src (not implemented yet)
#--redo                 will redo some operation that appear to be done
#--quiet                output from the stages is only in <stage-flv>.out
#--out                  (not done yet)
-v                     verbose

*B - build  stage option
*I - install stage option
"
reqarg='test -z "${1+1}" && echo opt $op requires arg. &&echo "$USAGE" && exit'
 op1arg='rest=`expr "$op" : "[^-]\(.*\)"`   && set --  "$rest" "$@";'"$reqarg"
 op1chr='rest=`expr "$op" : "[^-]\(.*\)"`   && set -- "-$rest" "$@"'
while [ "${1-}" ];do
    if expr "x${1-}" : 'x-' >/dev/null;then
        leq=`expr "x$1" : 'x--[^=]*=\(.*\)'` op=`echo "$1"|sed 's/^-//'`
        shift;test -n "$leq" &&set -- "$leq" "$@" && op=`expr "x$op" : 'x\([^=]*\)'`
        case "$op" in
        -prod)               eval $reqarg; opt_prod=$1; shift;;
        -ver)                eval $reqarg; opt_ver=$1; shift;;
        -prods-root)         eval $reqarg; opt_prods_root=$1; shift;;
        -productization-lib) eval $reqarg; opt_productization_lib=$1; shift;;
        -stages)             eval $reqarg; opt_stages=$1; shift;;
        -deps)               eval $reqarg; opt_deps="${opt_deps+$opt_deps,}$1"
                                                                        shift;;
        -var)                eval $reqarg; opt_var=$1; shift;;
        -configure)          eval $reqarg; opt_configure=$1; shift;;
        q*|-quals)           eval $op1arg; opt_quals=$1; shift;;

        v*)                  eval $op1chr; opt_v=`expr ${opt_v-0} + 1`;;
        -clean)                            opt_clean=1;;
        -redo)                             opt_redo=1;;
        -quiet)                            opt_quiet=1;;

        \?|h|-help)          echo "$USAGE";exit;;
        *)                   echo "Unknown option -$op";echo "$USAGE";exit;;
        esac
    else
        aa=`echo "$1" | sed -e "s/'/'\"'\"'/g"`
        args="${args-} '$aa'"; shift
    fi
done
eval "set -- ${args-} \"\$@\""

if [ $# -ne 1 ];then echo "no. arguments ($*) != 1";echo "$USAGE";exit;fi

cd $1
DIST_DIR=`pwd`

unset CDPATH   # CDPATH has been known to cause some configures and other
               # build mechanisms problems.
#-----------------------------------------------------------------------

if [ "${opt_quals-}" ];then PROD_QUALS=$opt_quals
else                        PROD_QUALS= ;fi

redo_stage=99
all_stages=build:install:install_fix:declare
if [ "${opt_stages-}" ];then
    stage_idx=0
    stages=
    for ss in `echo $all_stages | tr : ' '`;do
        stage_idx=`expr $stage_idx + 1`
        if xx=`expr ":${opt_stages}:" : ".*:\(${ss}\):"`;then
            stages="${stages:+$stages:}$xx"
            if [ $redo_stage -eq 99 ];then redo_stage=$stage_idx;fi
        fi
    done
else
    stages=$all_stages
fi

exec 3>&1 4>&2    # dup so as to enable restore
orestore='1>&3 2>&4'
if [ "${opt_out-}" ];then
    ospec='>>productize-`basename $PREFIX`${opt_qual+_`echo $opt_qual | tr : _`}.out 2>&1'
    # expect error status to be enough if error before exec after PROD_*
    exec >/dev/null 2>&1
else
    ospec='1>&3 2>&4'
fi

#-----------------------------------------------------------------------

echov() { if [ "${opt_v-}" ];then echo "`date`: $*";fi; }
cmd()   # used in ups_productization_lib.sh
{   eval "echov \"$@\""
    eval "$@"
    sts=$?
    if [ $sts -ne 0 ];then echo "Error with cmd: $*"; fi
    return $sts
}

#-----------------------------------------------------------------------

set_PRODS_RT()    # aka PRODS_DB (I combine them)
{
    if   [ "${opt_prods_root-}" ];then
        PRODS_RT="$opt_prods_root"
    elif [ "${PRODUCTS-}"       ];then
        DB=`expr "$PRODUCTS" : '\([^:]*\)'`
        if [ -f $DB/.upsfiles/dbconfig ];then
            # check for PROD_DIR_PREFIX
            PROD_DIR_PREFIX=`sed -n '/^[ \t]*PROD_DIR_PREFIX/{s/.*= *//;s/[ \t]*$//;p;}' $DB/.upsfiles/dbconfig`
            if [ "$PROD_DIR_PREFIX" = "$DB" -o "$PROD_DIR_PREFIX" = '${UPS_THIS_DB}' ];then
                PRODS_RT=$DB
            else
                echo "Incompatible DB $DB != $PROD_DIR_PREFIX"
            fi
        else
            echo 'Products DB, $DB, has not been initialized - .upsfile/dbconfig'
            echo 'file not found.'
        fi
    fi
    if [ "${PRODS_RT-}" = '' ];then
        echo 'Error - cannot determine "products root" - the place to where'
        echo 'product will be installed'
        echo "$USAGE"; exit
    fi
    echov PRODS_RT=$PRODS_RT
}   # set_PRODS_RT

set_NAM_and_VER()   # $1=nam-ver
{
    Pwd=`pwd`
    if nam_ver=`expr "$Pwd" : "$PRODS_RT/\([^/][^/]*/[^/][^/]*\)\$"`;then
        echo "name/ver=$nam_ver"
        prod_=`expr "$nam_ver" : '\([^/][^/]*\)'`
         ver_=`expr "$nam_ver" : '[^/][^/]*/\([^/][^/]*\)'`
        #stages=`echo :$stages:|sed 's/:build:/:/;s/:install:/:/;s/^://;s/:$//'`
        #echo "set stages=$stages"
    else
        # tests:
        #   gcc-4.1.2-20080102  -> gcc       v4_1_2_20080102
        #   root-v5-18-00f.svn  -> root      v5_18_00f_svn
        #   boost_1_34_1        -> boost     v1_34_1
        #   libsigc++-2.2.3     -> libsigcxx v2_2_3
        nam_ver=`basename $Pwd`
        prod_=`echo "$nam_ver" | sed 's/[-_]v*[0-9][-0-9.a-zA-Z]*$//'`
         ver_=`expr "$nam_ver" : "${prod_}[-_]\(.*\)"`
        prod_=`echo "$prod_" | sed 's/++/xx/'`    # libsigc++ -> libsigcxx
    fi

    if   [ "$prod_" = '' -a "${opt_prod-}" = '' ];then
        echo can not determine prod; return 1
    elif [ "$prod_"      -a "${opt_prod-}"      ];then
        if [ "$prod_" = "${opt_prod-}" ];then
            PROD_NAM=$prod_; echov OK - "$prod_" = "${opt_prod-}"
        else
            echov "Warning - $prod_ != ${opt_prod-}. Taking ${opt_prod-}."
            # example: prefer "python" over "Python" 
            PROD_NAM=$opt_prod
        fi
    else
        if [ "$prod_" ];then 
            PROD_NAM=$prod_;    echov "prod is $prod_"
        else
            PROD_NAM=$opt_prod; echov "prod is $opt_prod (from opt)"
        fi
    fi
    if   [ "$ver_" = '' -a "${opt_ver-}" = '' ];then
        echo "Error - can not determine ver (assuming prod=$PROD_NAM)"
        echo "   If prod is wrong, you might want to try using the --prod,"
        echo "--ver and/or --prods-root option(s). For example:"
        guess_root=`dirname $Pwd`
        echo " $0 --prods-root=`dirname $guess_root`"
        return 1
    elif [ "$ver_"      -a "${opt_ver-}"      ];then
        if [ "$ver_" = "${opt_ver-}" ];then
            PROD_VER=$ver_; echov OK - "$ver_" = "${opt_ver-}" 
        else
            echov "Warning - $ver_ != ${opt_ver-}. Taking ${opt_ver-}."
            PROD_VER=$opt_ver
        fi
    else
        if [ "$ver_" ];then
            PROD_VER=$ver_;    echov "ver is $ver_"
        else
            PROD_VER=$opt_ver; echov "ver is $opt_ver (from opt)"
        fi
    fi
    if expr "$PROD_VER" : '[0-9]' >/dev/null;then
        PROD_VER=v`echo "$PROD_VER" | tr '.\-' '__'`
    else
        PROD_VER=`echo "$PROD_VER" | tr '.\-' '__'`
    fi
}   # set_NAM_and_VER

#-----------------------------------------------------------------------

get_productization_lib()
{
    if [ $PROD_NAM = ups ];then
        PATH=`pwd`/bin:$PATH
        if [ -f bin/ups_productization_lib.sh ];then
            .   bin/ups_productization_lib.sh
        elif [ "${opt_productization_lib-}" ];then
            if [ -f "${opt_productization_lib-}" ];then
                . "${opt_productization_lib-}"
            else
                echo 'Error: --productization_lib is not a file'
                exit
            fi
        else
            echo "ups_productization_lib.sh not found"
            exit
        fi
    elif [ -d $PRODS_RT/ups ];then
        echov UPS_DIR =${UPS_DIR-}
        echov PRODS_RT=$PRODS_RT
        # I need _full_ ups (i.e. setup function)
        if [ -f $PRODS_RT/setup ];then
            . $PRODS_RT/setup
        else
            for fuefile in /usr/local/etc/fermi.shrc \
                   /usr/local/etc/setups.sh \
                   /fnal/ups/etc/setups.sh; do
                if [ -f $fuefile ];then
                    # I do not want to reset PRODUCTS
                    unset SETUP_UPS UPS_DIR # PRODUCTS
                    set +u  # bad programmers
                    . $fuefile
                    set -u
                    break
                fi
            done
            if type setup | grep 'ups setup' >/dev/null && hash ups 2>/dev/null;then
                : success
            else
                echo Error - can not initial ups support script environment
            fi
        fi
        if [ -f $UPS_DIR/bin/ups_productization_lib.sh ];then
            .   $UPS_DIR/bin/ups_productization_lib.sh
        else
            echo "ups_productization_lib.sh not found"
            exit
        fi
        # Now, additionally, there may be an  ${opt_productization_lib-} or
        # an {PROD_NAM}_productization_lib.sh
        for ff in ${opt_productization_lib+$opt_productization_lib $opt_productization_lib.sh} \
            ${PROD_NAM}_productization_lib \
            ${PROD_NAM}_productization_lib.sh;do
            for dd in '' .. $UPS_DIR/bin;do
              echov "checking for ${dd:+$dd/}$ff"
              if [ -f ${dd:+$dd/}$ff ];then . ${dd:+$dd/}$ff; break 2; fi
            done
        done
    else
        echo Error - ups must be installed 1st into products root at $PRODS_RT
    fi
}   # get_productization_lib

#------------------------------------------------------------------------------

set_PRODS_RT

if set_NAM_and_VER;then
    : OK
else
    exit 1
fi

# check write permissions...
# 

get_productization_lib

set_FLV

PROD_FQ=`flavor_qualifier_dir`
PREFIX=$PRODS_RT/$PROD_NAM/$PROD_VER/$PROD_FQ  # important var
BLD_DIR=$DIST_DIR/$PROD_FQ

echo
echo "  PRODS_RT=$PRODS_RT"
echo "  PROD_NAM=$PROD_NAM"
echo "  PROD_VER=$PROD_VER"
echo "  PROD_FLV=$PROD_FLV"
echo "  PROD_QUALS=$PROD_QUALS"
echo "  "
echo "  DIST_DIR=$DIST_DIR"     # if dist is src, dist_dir is src_dir
echo "  BLD_DIR=$BLD_DIR"                                # na if bin dist
echo "  "
echo "  PROD_FQ=$PROD_FQ"
echo "  INST_DIR=$PRODS_RT/$PROD_NAM/$PROD_VER/$PROD_FQ"
echo

#----

if [ "${opt_out-}" ];then
    >productize-`basename $PREFIX`${opt_qual+_`echo $opt_qual | tr : _`}.out
    eval "exec ${ospec-}"
fi
Pwd=`pwd`
echov "$APP: starting in .../`basename $Pwd`"

if   declare     --status;then  last_stage_done=4
elif install_fix --status;then  last_stage_done=3
elif install     --status;then  last_stage_done=2
elif build       --status;then  last_stage_done=1
else                            last_stage_done=0
fi
if [ "${opt_quiet-}" ];then outspec='>/dev/null 2>&1';else outspec=; fi
echov last_stage_done=$last_stage_done redo_stage=$redo_stage stages=$stages
for ss in `echo $stages | tr : ' '`;do
    #if echo $stages | grep $ss >/dev/null;then :;else continue; fi
    # all stages are run in a pipeline to collect output AND to run in
    # sub-shell (so the cwd in this parent shell is preserved).
    pid=
    ofile=$ss-`basename $PREFIX`${opt_qual+_`echo $opt_qual | tr : _`}.out
    case $ss in
    build)
        if [ $last_stage_done -lt 1 -o $redo_stage -le 1 ];then
            echo "$APP: output in $ofile"
            exec >$ofile 2>&1
            build & pid=$!
        fi;;
    install)
        if [ $last_stage_done -lt 2 -o $redo_stage -le 2 ];then
            echo "$APP: output in $ofile"
            exec >$ofile 2>&1
            install & pid=$!
        fi;;
    install_fix)
        if [ $last_stage_done -lt 3 -o $redo_stage -le 3 ];then
            echo "$APP: output in $ofile"
            exec >$ofile 2>&1
            install_fix & pid=$!
        fi;;
    declare)
        if [ $last_stage_done -lt 4 -o $redo_stage -le 4 ];then
            echo "$APP: output in $ofile"
            exec >$ofile 2>&1
            declare & pid=$!
        fi;;
    esac
    if [ "$pid" ];then
        eval "exec ${ospec-}"  # restore normal output
        if [ "${opt_v-}" ];then
            tail --pid=$pid -f $ss-`basename $PREFIX`.out
            wait $pid
        else
            wait $pid
        fi
        if [ $? -eq 0 ];then
            echov "stage $ss completed OK"
        else
            echo "Error in $ss stage for $PROD_NAM $PROD_VER ${opt_qual-}"
            exit 1            
        fi
    fi
done

Pwd=`pwd`
echov "Done in .../`basename $Pwd`"
