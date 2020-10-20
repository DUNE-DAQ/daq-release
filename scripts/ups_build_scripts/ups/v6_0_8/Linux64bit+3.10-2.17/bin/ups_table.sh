#! /bin/sh
#  This file (ups_table.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
#  Jul 16, 2009. "TERMS AND CONDITIONS" governing this file are in the README
#  or COPYING file. If you do not have such a file, one can be obtained by
#  contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#  $RCSfile: ups_table.sh,v $
#  rev='$Revision: 1.7 $$Date: 2009/12/06 04:25:05 $'

USAGE="\
  The purpose of this script is to automatically adjust the generic
table file to add qualified (or build) and non-qualified dependencies
and qualified (or build) and non-qualified env. variables.

   usage: `basename $0` <cmd=add_fq> [options]
examples: `basename $0` add_fq
          `basename $0` add_fq -f Linux+2.6-2.5 --envvar=OSPL_FQ=HDE_gcc41/x86.linux2.6
options:
  -f flavor
  -q qualifiers
  -m table file
 --deps=    comma separated list of dependent products to setup before the
            generic setup action is executed.
 --envini=  initial env.vars set before generic setup action is
            executed. (comma  separated list: VAR=val[,VAR=val]...)
 --envvar=  comma separated list: VAR=val[,VAR=val]...  (set after generic
            setup action is executed.)
 --envpre=  comma separated list simalar to --envvar but value get prepended to
               the existing variable list value (if defined or just assigned as
               the variable value if not already defined).
 --alias=   comma separated list: alias_name=alias_val[,alias_name=alias_val]...
# --envvar-com=  set at end of generic setup action
# --envpre-com=  set at end of generic setup action
# - not implemented yet.
"
set_opt()
{
reqarg='test -z "${1+1}" && echo opt $op requires arg. &&echo "$USAGE" && exit'
 op1arg='rest=`expr "$op" : "[^-]\(.*\)"`   && set --  "$rest" "$@";'"$reqarg"
 op1chr='rest=`expr "$op" : "[^-]\(.*\)"`   && set -- "-$rest" "$@"'
    while expr "x${1-}" : 'x-' >/dev/null;do
        leq=`expr "x$1" : 'x--[^=]*=\(.*\)'` op=`echo "$1"|sed 's/^-//'`
        shift;test "$leq" &&set -- "$leq" "$@" && op=`expr "$op" : '\([^=]*\)'`
        case "$op" in
        m*)               eval $op1arg; opt_tblf=$1; shift;;
        f*)               eval $op1arg; opt_flv=$1; shift;;
        q*)               eval $op1arg; opt_qual=$1; shift;;
        -deps)   eval $reqarg;opt_deps="${opt_deps+$opt_deps,}$1";shift;;
        -envvar) eval $reqarg;opt_envvar="${opt_envvar+$opt_envvar,}$1";shift;;
        -alias)  eval $reqarg;opt_alias="${opt_alias+$opt_alias,}$1";shift;;
        -envpre) eval $reqarg;opt_envpre="${opt_envpre+$opt_envpre,}$1";shift;;
        -envini) eval $reqarg;opt_envini="${opt_envini+$opt_envini,}$1";shift;;
        -env)    eval $reqarg;opt_env="${opt_env+$opt_env,}$1";shift;;

        v*)                  eval $op1chr; opt_v=`expr ${opt_v-0} + 1`;;

        \?|h|-help)          echo "$USAGE";exit;;
        *)                   echo "Unknown option -$op";echo "$USAGE";exit;;
        esac
    done
}

echov() { if [ "${opt_v-}" ];then echo "$@" >&2; fi; }

#-------------------------------------------------------------------------

build_fq_ere()
{   list=$1
    pre_list=${2-}
    for qq in `echo $list | tr : ' '`;do
        new_list=`echo :$list: | sed "s/:$qq:/:/;s/^://;s/:$//"`
        if [ "${opt_all-}" ];then echo "$pre_list$qq"; fi
        if [ "$new_list" ];then
            yy=`build_fq_ere "$new_list" "$pre_list${qq}:"`
            echo "$yy"
        else
            if [ "${opt_all-}" = '' ];then echo "$pre_list$qq"; fi
        fi
    done
}

#-------------------------------------------------------------------------
# add the 
add_fq()
{
    set_opt "$@"
    set_TBLF
    set_FLV
    set_QUAL
    if [ "$QUAL" ];then
        qual_ere="(`build_fq_ere $QUAL | sed -n 'H;${x;s/\n/|/g;s/^|//;p;}'`)"
    else
        qual_ere=
    fi
    echov "qual_ere=$qual_ere"
    if list_fq | egrep "$FLV \"$qual_ere\"";then
        echov "flv/qual combination already exists"
        awk_fq='{print;}'
    else
        awk_fq="BEGIN{IGNORECASE=1;}
{ print; }
/^GROUP:/{ printf \"\n\
FLAVOR = $FLV\nQUALIFIERS = \\\"$QUAL\\\"\n\
    ACTION = setup\n\
        exeActionRequired(\\\"setup__\\\")\n\
\";}"
    fi

    # insert any depend product setups just AFTER the "ACTION = setup" line
    # that is added above
    if [ "${opt_deps-}" ];then
        sed_up=''
        sed_dn=''
        IFSsav=$IFS IFS=,; for dep in $opt_deps;do IFS=$IFSsav
            dep=`echo $dep`  # removes potential space next to comma IFS
            sed_up="$sed_up        setupRequired( \\\"$dep\\\" )\\n"
            # unsetup in opposite order
            sed_dn="\\n        unsetupRequired( \"$dep\" )$sed_dn"
        done
        awk_deps="BEGIN{IGNORECASE=1;}
 {print;}
 /^flavor *=/{flv=gensub(/flavor *= */,\"\",1);}
 /^qualifiers *=/{qual=gensub(/qualifiers *= */,\"\",1);
   if (flv == \"$FLV\" && qual == \"\\\"$QUAL\\\"\") {
      do_it=1;
   }
 }
 /^ *action *= *setup/ {
   if (do_it == 1) {printf \"$sed_up\";do_it=0;}
 }
"
    else
        awk_deps='{print;}'
    fi

    # insert any variable definitions BEFORE the ``exeActionRequired("setup__")'' line
    if [ "${opt_envini-}" ];then
        envvar=''
        IFSsav=$IFS IFS=,; for vv in $opt_envini;do IFS=$IFSsav
            vvar=`expr "$vv" : ' *\([^=]*\)'` # recall, there may be a " " after a "," IFS
            vval=`expr "$vv" : '[^=]*=\(.*\)'`
            envvar="$envvar        envSet( $vvar, $vval )\\n"
        done
        awk_envini="BEGIN{IGNORECASE=1;}
 /^flavor *=/{flv=gensub(/flavor *= */,\"\",1);}
 /^qualifiers *=/{qual=gensub(/qualifiers *= */,\"\",1);
   if (flv == \"$FLV\" && qual == \"\\\"$QUAL\\\"\") {
      do_it=1;
   }
 }
 /^ *exeActionRequired/ {
   if (do_it == 1) {printf \"$envvar\";do_it=0;}
 }
 {print;}
"
    else
        awk_envini='{print;}'
    fi

    # insert any variable definitions AFTER the ``exeActionRequired("setup__")'' line
    if [ "${opt_envvar-}" ];then
        envvar=''
        IFSsav=$IFS IFS=,; for vv in $opt_envvar;do IFS=$IFSsav
            vvar=`expr "$vv" : ' *\([^=]*\)'` # recall, there may be a " " after a "," IFS
            vval=`expr "$vv" : '[^=]*=\(.*\)'`
            envvar="$envvar        envSet( $vvar, $vval )\\n"
        done
        awk_envvar="BEGIN{IGNORECASE=1;}
 /^flavor *=/{flv=gensub(/flavor *= */,\"\",1);}
 /^qualifiers *=/{qual=gensub(/qualifiers *= */,\"\",1);
   if (flv == \"$FLV\" && qual == \"\\\"$QUAL\\\"\") {
      do_it=1;
   }
 }
 {print;}
 /^ *exeActionRequired/ {
   if (do_it == 1) {printf \"$envvar\";do_it=0;}
 }
"
    else
        awk_envvar='{print;}'
    fi

    # insert any envPrepend definitions AFTER the ``exeActionRequired("setup__")'' line
    if [ "${opt_envpre-}" ];then
        envvar=''
        IFSsav=$IFS IFS=,; for vv in $opt_envpre;do IFS=$IFSsav
            vvar=`expr "$vv" : ' *\([^=]*\)'` # recall, there may be a " " after a "," IFS
            vval=`expr "$vv" : '[^=]*=\(.*\)'`
            envvar="$envvar        envPrepend( $vvar, $vval )\\n"
        done
        awk_envpre="BEGIN{IGNORECASE=1;}
 /^flavor *=/{flv=gensub(/flavor *= */,\"\",1);}
 /^qualifiers *=/{qual=gensub(/qualifiers *= */,\"\",1);
   if (flv == \"$FLV\" && qual == \"\\\"$QUAL\\\"\") {
      do_it=1;
   }
 }
 {print;}
 /^ *exeActionRequired/ {
   if (do_it == 1) {printf \"$envvar\";do_it=0;}
 }
"
    else
        awk_envpre='{print;}'
    fi

    # insert any alias definitions AFTER the ``exeActionRequired("setup__")'' line
    if [ "${opt_alias-}" ];then
        alias=''
        IFSsav=$IFS IFS=,; for vv in $opt_alias;do IFS=$IFSsav
            anam=`expr "$vv" : ' *\([^=]*\)'` # recall, there may be a " " after a "," IFS
            adef=`expr "$vv" : '[^=]*=\(.*\)'`
            alias="$alias        addAlias( $anam, $adef )\\n"
        done
        awk_alias="BEGIN{IGNORECASE=1;}
 /^flavor *=/{flv=gensub(/flavor *= */,\"\",1);}
 /^qualifiers *=/{qual=gensub(/qualifiers *= */,\"\",1);
   if (flv == \"$FLV\" && qual == \"\\\"$QUAL\\\"\") {
      do_it=1;
   }
 }
 {print;}
 /^ *exeActionRequired/ {
   if (do_it == 1) {printf \"$alias\";do_it=0;}
 }
"
    else
        awk_alias='{print;}'
    fi

    # because envvar, envpre, and aliases are all inserted AFTER the
    # ``exeActionRequired("setup__")'' line, the order matters;
    # they will appear in the output in the reverse order that they are in in
    # the pipeline
    cat $TBLF | awk "$awk_fq"     | awk "$awk_deps"   | awk "$awk_envini" \
              | awk "$awk_alias"  | awk "$awk_envpre" | awk "$awk_envvar"
}   # add_fq

list_fq()
{
    set_opt "$@"
    set_TBLF
    awk 'BEGIN{IGNORECASE=1;}
 /^flavor *=/{flv=gensub(/flavor *= */,"",1);}
 /^qualifiers *=/{qual=gensub(/qualifiers *= */,"",1);
   printf "%s %s\n",flv,qual;
 }
' $TBLF
}

list_vars()
{
    set_opt "$@"
    set_TBLF
    set_FLV
    set_QUAL
    awk "BEGIN{IGNORECASE=1;}
 /^flavor *=/{flv=gensub(/flavor *= */,\"\",1);do_it=0;}
 /^qualifiers *=/{qual=gensub(/qualifiers *= */,\"\",1);
   if (flv == \"$FLV\" && qual == \"\\\"$QUAL\\\"\") {
      do_it=1;
   }
 }
 /^common:/{do_it=0;}
 /^ *envset/ {
   if (do_it == 1) {printf \"%s\\n\",gensub(/^ *envset *\\( *([^ ,]*) *, *(.*)\\)$/,\"\\\\1=\\\\2\",1);}
 }
" $TBLF
}

list_deps()
{
    set_opt "$@"
    set_TBLF
    set_FLV
    set_QUAL
    awk "BEGIN{IGNORECASE=1;}
 /^flavor *=/{flv=gensub(/flavor *= */,\"\",1);do_it=0;}
 /^qualifiers *=/{qual=gensub(/qualifiers *= */,\"\",1);
   if (flv == \"$FLV\" && qual == \"\\\"$QUAL\\\"\") {
      do_it=1;
   }
 }
 /^common:/{do_it=0;}
 /^ *setuprequired/ {
   if (do_it == 1) {printf \"%s\\n\",gensub(/^ *setuprequired *\\( *\"([^\"]*)\" *\\).*$/,\"\\\\1\",1);}
 }
" $TBLF
}

#------------------------------------------------------------------------------
set_TBLF()
{   if [ "${TBLF-}" ];then return; fi
    if [ "${opt_tblf-}" ];then TBLF=$opt_tblf
    else                       TBLF=$UPS_DIR/ups/generic.table; fi
}
set_FLV()
{   if [ "${FLV-}" ];then return; fi
    if [ "${opt_flv-}" ];then FLV=$opt_flv
    else                      FLV=`ups flavor`; fi
}
set_QUAL()
{   if [ "${QUAL-}"     ];then return; fi
    if [ "${opt_qual-}" ];then QUAL=$opt_qual
    else                       QUAL=; fi
}
#------------------------------------------------------------------------------

if [ "${1-}" = -x ];then set -x;shift; fi
if [ "${1-}" ];then
    set -u
    cmd=$1; shift
    case $cmd in
    add_fq)    add_fq "$@";;
    list_fq)   list_fq "$@";;
    list_vars) list_vars "$@";;
    list_deps) list_deps "$@";;
    *)       echo "Unknown cmd $cmd."; echo "$USAGE"; exit 1;;
    esac
fi
