#! /bin/sh
#  This file (ups_declare.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
#  Sep  1, 2009. "TERMS AND CONDITIONS" governing this file are in the README
#  or COPYING file. If you do not have such a file, one can be obtained by
#  contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#  $RCSfile: ups_declare.sh,v $
#  rev='$Revision: 1.8 $$Date: 2010/06/18 15:25:25 $'
if [ "$1" = -x ];then set -x;shift; fi
set -u

app=`basename $0`
opts_wo_args='v|n|ups|y'
opts_w_args='deps|envvar|envpre|alias'
USAGE="\
Declare a product rooted within a ups combined ups database/products root.

usage: $app [options] <dir>...
options=$opts_wo_args         (do not have option-arguments)
     or $opts_w_args    (require option-arguments)

examples:
  1 \$ $app --ups .
  2 \$ xx='\\\${UPS_PROD_DIR}/\\\$\\\${UPS_PROD_NAME_UC}_FQ';\\
$app --ups --envpre=\"\\
RUBYLIB=\$xx/lib/ruby/1.9.1/x86_64-linux,\\
RUBYLIB=\$xx/lib/ruby/1.9.1,\\
RUBYLIB=\$xx/lib/ruby/vendor_ruby,\\
RUBYLIB=\$xx/lib/ruby/vendor_ruby/1.9.1/x86_64-linux,\\
RUBYLIB=\$xx/lib/ruby/vendor_ruby/1.9.1,\\
RUBYLIB=\$xx/lib/ruby/site_ruby,\\
RUBYLIB=\$xx/lib/ruby/site_ruby/1.9.1/x86_64-linux,\\
RUBYLIB=\$xx/lib/ruby/site_ruby/1.9.1\\
\" .
  3 \$ $app --ups --envvar=\"\\
OSPL_HOME=\\\\\\\${UPS_PROD_DIR}/\\\\\\\$OSPL_FQ,\\
OSPL_TARGET=x86.linux2.6,\\
CPATH=\\\\\\\$OSPL_HOME/include:\\\\\\\$OSPL_HOME/include/sys,\\
OSPL_TMPL_PATH=\\\\\\\$OSPL_HOME/etc/idlpp,\\
OSPL_URI=file://\\\\\\\$OSPL_HOME/etc/config/ospl.xml,\\
PTECH_LICENSE_FILE=\\\\\\\$OSPL_HOME/etc\"\\
 --envpre=\"\\
CLASSPATH=\\\\\\\$OSPL_HOME/jar/dcpssaj.jar,\\
CLASSPATH=\\\\\\\$OSPL_HOME/jar/dcpscj.jar,\\
CLASSPATH=\\\\\\\$OSPL_HOME/jar/dlrlsaj.jar\"\\ .
"
do_opt="\
  case \$opt in
  \\?|h|help)    echo \"\$USAGE\"; exit 0;;
  $opts_wo_args) vv=opt_\`echo \$opt |sed -e 's/-/_/g'\`
                 eval xx=\\\${\$vv-0};eval \"\$vv=\`expr \$xx + 1\`\";;
  $opts_w_args)  if   [ \"\${rest-}\" != '' ];then arg=\$rest; rest=
                 elif [      $#      -ge 1  ];then arg=\$1; shift
                 else  echo \"option \$opt requires argument\"; exit 1; fi
                 eval opt_\`echo \$opt|sed -e 's/-/_/g'\`=\"'\$arg'\"
                 opts=\"\$opts '\$arg'\";;      # tricky part Aa
  *) echo \"invalid option: \$opt\" 2>&1;echo \"\$USAGE\" 2>&1; exit 1;;
  esac"
while op=`expr "${1-}" : '-\(.*\)'`;do
    shift
    if xx=`expr "$op" : '-\(.*\)'`;then
        if   opt=`expr     "$xx" : '\([^=]*\)='`;then
            set ${-+-$-} -- "`expr "$xx" : '[^=]*=\(.*\)'`" "$@"
        else opt=$xx; fi
        opts="${opts-} '--$opt'"
        eval "$do_opt"
    else
        while opt=`expr "$op" : '\(.\)'`;do
            opts="${opts-} '-$opt'"
            rest=`expr "$op" : '.\(.*\)'`
            eval "$do_opt"
            op=$rest
        done
    fi
done

if [ $# -lt 1 ];then echo "$USAGE";exit; fi

if [ "${opt_y-}" = '' ];then
    echo '# No commands will be executed. Use -y options to cause command execution.'
fi

#-----------------------------------------------------------------------

echov() { if [ "${opt_v-}" ];then echo "#`date`; $@";fi; }
cmd()
{
    if [ "${opt_y-}" ];then
        echo "executing $@..."
        eval "$@"
    else
        echo "$@"
    fi
}

#-----------------------------------------------------------------------

set_PRODS_RT()    # aka PRODS_DB (I combine them)
{
    dd=$PWD
    PRODS_RT=
    while [ $dd != / ];do
        DB=$dd
        if [ -f $DB/.upsfiles/dbconfig ];then
            # check for PROD_DIR_PREFIX
            PROD_DIR_PREFIX=`sed -n '/^[ \t]*PROD_DIR_PREFIX/{s/.*= *//;s/[ \t]*$//;p;}' $DB/.upsfiles/dbconfig`
            if [ "$PROD_DIR_PREFIX" = "$DB" -o "$PROD_DIR_PREFIX" = '${UPS_THIS_DB}' ];then
                PRODS_RT=$DB
                PRODS_DB=$DB
                break
            fi
        fi
        dd=`dirname $dd`
    done
    if [ ! -f $DB/.upsfiles/dbconfig ];then
        echo "Incompatible UPS structure -- products root not combined within a ups db."
        exit 1
    elif [ "$PRODS_RT" = '' ];then
        echo "Incompatible DB ($DB?) != ${PROD_DIR_PREFIX-}"
        exit 1
    fi
    echov PRODS_RT=$PRODS_RT
}   # set_PRODS_RT

set_NAM_and_VER()   # $1=nam-ver
{
    if nam_ver=`expr "$PWD" : "$PRODS_RT/\([^/][^/]*/[^/][^/]*\)"`;then
        echov "name/ver=$nam_ver"
        prod_=`expr "$nam_ver" : '\([^/][^/]*\)'`
         ver_=`expr "$nam_ver" : '[^/][^/]*/\([^/][^/]*\)'`
    else
        echo 'Error: not in prod_root/prod/version directory' >&2
        exit 1
    fi
    PROD_NAM=$prod_;    echov "prod is $prod_"
    PROD_VER=$ver_;    echov "ver is $ver_"
}   # set_NAM_and_VER

#-----------------------------------------------------------------------

for dd in "$@";do
    echo "#$dd"
    cd $dd

    set_PRODS_RT

    set_NAM_and_VER

    cd $PRODS_RT/$PROD_NAM/$PROD_VER

    ups_flavor.sh ${opt_ups:+--ups} 2>/dev/null | while read flavor fq;do
        echo "#    $flavor $fq"
        PROD_FLV=`expr $flavor : 'flavor=\(.*\)'`
        PROD_FQ=`expr $fq : 'fq=\(.*\)'`

        opt_K="PRODUCT:VERSION:FLAVOR:QUALIFIERS:CHAIN:PROD_DIR"
        ll=`ups list -aK$opt_K -z$PRODS_DB -f$PROD_FLV $PROD_NAM $PROD_VER`
        if [ -n "$ll" ];then
            echo "#    prod exists: $ll"
            continue
        fi

        if [ ! -d ups ];then cmd "    mkdir ups"; fi
        if   [ -f ups/$PROD_NAM.table ];then prd_has_upstbl=y tblf=$PROD_NAM.table
        elif [ -f ups/generic.table   ];then prd_has_upstbl=y tblf=generic.table
        else
            # need to have a table file from which to copy from
            if [ ! -f $UPS_DIR/ups/generic.table ];then
                echo "Cannot find \$UPS_DIR/generic.table."
                echo "Skipping ups declare for $PROD_NAM $PROD_VER."
                echo "Please try to generate a table file."
                continue
            else
                prd_has_upstbl=n tblf=$PROD_NAM.table # create $PROD_NAM.table
            fi
        fi

        if   qq=`expr "$PROD_FQ" : "${PROD_FLV}_\(.*\)"`;then
            QUAL="-q`echo $qq | tr _ :`"
            if [ "$PROD_FQ" != "$PROD_FLV" ];then
                prod_name_uc=`echo $PROD_NAM | tr '[a-z]' '[A-Z]'`
                INI="--envini=${prod_name_uc}_FQ=$PROD_FQ${opt_envini+,$opt_envini}"
            else
                INI=${opt_envvar+--envvar=$opt_envvar}
            fi
        elif [ "$PROD_FQ" != . -a "$PROD_FQ" != "$PROD_FLV" ];then
            prod_name_uc=`echo $PROD_NAM | tr '[a-z]' '[A-Z]'`
            INI="--envini=${prod_name_uc}_FQ=$PROD_FQ${opt_envini+,$opt_envini}"
        else
            INI=${opt_envvar+--envvar=$opt_envvar}
        fi


        if [    -z "${QUAL-}"     -a -z "${INI-}"\
             -a -z "${opt_deps-}" -a -z "${opt_alias-}"\
             -a -z "${opt_envpre-}" ];then
            if [ "$prd_has_upstbl" = y ];then
                echo "# tblf OK"
            else
                cmd "    cp $UPS_DIR/ups/generic.table ups/$tblf"
            fi
        else
            if [ "$prd_has_upstbl" = y ];then
                cmd "    mv ups/$tblf{,.old}"
                tbl_src_opt="-mups/$tblf.old"
            else
                tbl_src_opt=
            fi
            cmd "    ups_table.sh add_fq $tbl_src_opt -f $PROD_FLV ${QUAL-}\
 ${opt_deps+--deps=\"$opt_deps\"}\
 ${opt_alias+--alias=\"$opt_alias\"}\
 ${opt_envpre+--envpre=\"$opt_envpre\"}\
 ${opt_envvar+--envvar=\"$opt_envvar\"}\
 ${INI+\"$INI\"} >ups/$tblf"
        fi

        declare="ups declare -c -z$PRODS_RT -r$PROD_NAM/$PROD_VER -Mups"
        declare="$declare -m$tblf -f$PROD_FLV $PROD_NAM $PROD_VER"
        cmd "    $declare"
    done
done
