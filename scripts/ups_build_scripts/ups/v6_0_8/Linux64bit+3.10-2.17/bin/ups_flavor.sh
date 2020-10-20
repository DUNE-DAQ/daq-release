#! /bin/sh
#  This file (ups_flavor.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
#  Aug 21, 2009. "TERMS AND CONDITIONS" governing this file are in the README
#  or COPYING file. If you do not have such a file, one can be obtained by
#  contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#  $RCSfile: ups_flavor.sh,v $
#  $Revision: 1.11 $
#  $Date: 2010/06/29 16:20:40 $
if [ "$1" = -x ];then set -x; shift; fi
set -u

VERS="GLIBC_ GLIBCXX_ CXXABI_" # no fortran yet
tab=`echo -e "\t"`
#echo "tab=>$tab<"

APP=`basename $0`
USAGE="\
Attempt to tell the \"flavor\" of the bin and lib sub directories based on
the info from the files in those directories.  Or, just print the recommend
Linux flavor for the host for use in a ups declare.
  usage: $APP [-?|-v|--ups] [dir]
    OR   $APP --host --ups
example: cd some/dir
         $APP
"
opts_wo_args='v|ups|host'
do_opt="\
  case \$opt in
  \\?|h|help)    echo \"\$USAGE\"; exit 0;;
  $opts_wo_args) vv=opt_\`echo \$opt |sed -e 's/-/_/g'\`
                 eval xx=\\\${\$vv-0};eval \"\$vv=\`expr \$xx + 1\`\";;
  *) echo \"invalid option: \$opt\" 2>&1;echo \"\$USAGE\" 2>&1; exit 1;;
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

if [ $# -gt 1 ];then echo "$USAGE";exit; fi
if [ $# -eq 1 ];then cd $1 >/dev/null; fi

if [ -n "${opt_host-}" ];then
    flv=`ups flavor`
    if [ -n "${opt_ups-}" ];then
        case $flv in
        Linux*+2.4*)     flv=`expr $flv : '\([^+]*\)'`;;
        Linux*+2.6-2.3*) flv=`expr $flv : '\([^-]*\)'`;;
        Linux*+2.6-2.5*) : let it be;;
        esac
    fi
    echo $flv
    exit
fi

echov()  { if [ ${opt_v-0} -ge 1 ];then echo "$@"; fi; }
echovv() { if [ ${opt_v-0} -ge 2 ];then echo "$@"; fi; }

dirsOfInterest=`find . -type d \( -name bin -o -name 'bin.*' \)`
dirsOfInterest="$dirsOfInterest `find . -type d \( -name lib -o -name 'lib.*' \)`"
if [ "$dirsOfInterest" = ' ' ];then dirsOfInterest=.;fi

adjust_os()
{
    echo $1 | sed 's|GNU/||'
}

dot2dec()
{  ee=`echo $1.0.0 | sed \
   's/\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\).*/\1 \\\* 65536 + \2 \\\* 256 + \3/'`
   eval "expr $ee"
}
dec2dot()
{   d1=`expr $1 / 65536`
    d2=`expr \( $1 - \( $d1 \* 65536 \) \) / 256`
    d3=`expr $1 - $d1 \* 65536 - $d2 \* 256`
    echo $d1.$d2.$d3
}

awkvers()
{
    awk "/$1[1-9]/{
  xx=\$0;while(ii=match(xx,/$1([0-9.]*)/,aa)){
   sub(/$1[0-9.]*/,\"\",xx);print aa[1];
   }
 }"
}
awkhilo()
{
    awk 'BEGIN{lo=0xffffff;hi=0;}
  { xx=$0 ".0.0";
    x1=gensub(/([^.]*).*/,"\\1",1,xx);
    x2=gensub(/[^.]*\.([^.]*).*/,"\\1",1,xx);
    x3=gensub(/[^.]*\.[^.]*\.([^.]*).*/,"\\1",1,xx);
    xo=x1*65536+x2*256+x3;
    if (xo < lo) lo=xo;
    if (xo > hi) hi=xo;
    #printf("%d 0x%x %s %s %s\n",xo,xo,x1,x2,x3);
  }
  END{
    #l1=lo/65536;l2=and(lo,0xff00)/256;l3=and(lo,0xff);
    #h1=hi/65536;h2=and(hi,0xff00)/256;h3=and(hi,0xff);
    #printf("%d   %d.%d.%d  %d  %d.%d.%d\n",lo,l1,l2,l3,hi,h1,h2,h3);
    printf("%d %d\n",lo,hi);
  }'
}

fq_list=
add_to_fq_list()
{
    add_fq=$1;   shift
    add_dd=$1;   shift
    add_os=$1;   shift
    add_bits=$1; shift
    echov "add_fq=$add_fq add_dd=$add_dd add_os=$add_os add_bits=$add_bits $*"
    if _fq=`echo "$fq_list" | grep "^$add_fq$tab"`;then
        fq_list=`echo "$fq_list" | grep -v "^$add_fq$tab"`    # remove the existing one from the list
        #echov fq_list grep-v lines: `echo "$fq_list" | wc -lc`
        echov "fq_list: $fq_list"
        # prv is for "previous" (the fq we just removed)
        prv_dd=`echo "$_fq" | awk 'BEGIN{FS="\t";}{print $2;}'`
        prv_os=`echo "$_fq" | awk 'BEGIN{FS="\t";}{print $3;}'`
        if [ $prv_os != ANY ];then add_os=$prv_os;fi
        prv_bits=`echo "$_fq" | awk 'BEGIN{FS="\t";}{print $4;}'`
        if [ $prv_bits -gt $add_bits ];then add_bits=$prv_bits;fi
        add_str=
        str_idx=5
        prv=`echo "$_fq" | awk "BEGIN{FS=\"\\\t\";}{print \\\$$str_idx;}"`
        yy=$1;shift   # should be klo
        if [ $yy -lt $prv ];then prv=$yy;fi
        add_str="$prv"
        str_idx=`expr $str_idx + 1`
        for vv in $VERS;do
            prv=`echo "$_fq" | awk "BEGIN{FS=\"\\\t\";}{print \\\$$str_idx;}"`
            yy=$1;shift
            if [ $yy -gt $prv ];then prv=$yy;fi
            add_str="$add_str${tab}$prv"
            str_idx=`expr $str_idx + 1`
        done
        # check for any weirdness in the new values (and just stick with the old in that case)
        #  got_plt  hash   gnu_hash
        #     0       0       0         invalid
        #     0       0       1         invalid
        #     0       1       0        Linux     +2.4-2.3.2    i.e. LTS3
        #     0       1       1         invalid
        #     1       0       0         invalid
        #     1       0       1        Linux     +2.6-2.5      i.e. SL5
        #     1       1       0        Linux     +2.6-2.3.4    i.e. SL4
        #     1       1       1         invalid
        prv=`echo "$_fq" | cut -d"$tab" -f$str_idx-`
        yy="$1$tab$2$tab$3"
        if [ "$1 $2 $3" = '0 0 0' ];then yy=$prv;fi
        if [ "$prv" != "$yy" ];then echo "Strange: \"$prv\" != \"$yy\"" >&2;fi
        add_str="$add_str${tab}$yy"
        if [ "$fq_list" ];then
            fq_list=`echo "$fq_list";echo "$add_fq${tab}$add_dd${tab}$add_os${tab}$add_bits${tab}$add_str"`
        else
            fq_list="$add_fq${tab}$add_dd${tab}$add_os${tab}$add_bits${tab}$add_str"
        fi
    elif _fq=`echo "$fq_list" | grep "${tab}$add_dd${tab}"`;then
        echov found $add_dd
        if [ $add_os != ANY ];then
            fq_list=`echo "$fq_list" | grep -v "^$add_fq$tab"`
            if [ "$fq_list" ];then
                fq_list=`echo "$fq_list";echo "$add_fq${tab}$add_dd${tab}$add_os${tab}$add_bits${tab}$1${tab}$2${tab}$3${tab}$4${tab}$5${tab}$6${tab}$7"`
            else
                fq_list="$add_fq${tab}$add_dd${tab}$add_os${tab}$add_bits${tab}$1${tab}$2${tab}$3${tab}$4${tab}$5${tab}$6${tab}$7"
            fi
        fi
    elif [ "$fq_list" ];then
        fq_list=`echo "$fq_list";echo "$add_fq${tab}$add_dd${tab}$add_os${tab}$add_bits${tab}$1${tab}$2${tab}$3${tab}$4${tab}$5${tab}$6${tab}$7"`
    else
        fq_list="$add_fq${tab}$add_dd${tab}$add_os${tab}$add_bits${tab}$1${tab}$2${tab}$3${tab}$4${tab}$5${tab}$6${tab}$7"
    fi
    echov fq_list lines: `echo "$fq_list" | wc -lc`
}   # add_to_fq_list

cnt_f=0
for dd in $dirsOfInterest;do
    # 3 cases for fq
    if   [ \( `basename $dd` = bin -o `basename $dd` = lib \) \
        -a `dirname $dd` != . ];then
        # fq/bin
        fq=`dirname $dd`; fq=`expr "$fq" : './\(.*\)'`
    elif fq=`expr "$dd" : '.*/bin\.\(.*\)'` || fq=`expr "$dd" : '.*/lib\.\(.*\)'`;then
        # bin.fq   --- assuming NOT blah/blah/bin.fq
        :
    else
        # bin/fq
        fq=   # to be determined below
    fi
    pushd $dd >/dev/null
    if [ "`dirname $dd`" = . ];then
        # must search subdirs -- fq should be ''
        subdirs=`find . -type d \! -name CVS \! -name .svn -print`
    else
        # should alread have fq -- avoid fq/bin/fq
        subdirs="."
    fi
    # "." will be the 1st one -- 
    # if "." has a flavor, then we should assume the layout:
    #      flv1/lib
    #          /bin
    #      flv2/lib
    #          /bin
    # if "." does not have a flavor, then we should assume the layout:
    #      lib/flv1
    #         /flv2
    #      bin/flv1
    #          flv2
    fq_=$fq
    for ss in $subdirs;do
        if [ -z "$fq_" ];then
            fq=`expr $ss : '\./*\(.*\)'`
            # note: fq will be '' for single flavor ./bin/file (non-fq) structure
            if [ "$fq" = '' ];then fq=.;fi
        fi
           fq_dd=`expr "$dd/$ss" : "./$fq/bin/\(.*\)"` \
        || fq_dd=`expr "$dd/$ss" : "./$fq/lib/\(.*\)"` \
        || fq_dd=`expr "$dd/$ss" : "\(.*\)/bin\.$fq"` \
        || fq_dd=`expr "$dd/$ss" : "\(.*\)/lib\.$fq"` \
        || fq_dd=.
        echov "potential fq=$fq"
        pushd $ss >/dev/null
        files=`find . \! -name . -prune -type f -print`
        if [ "$files" ];then
            # 16777215  ==  0xffffff
            os=ANY           os_file=
            bits=0           bits_file=
            prbsl4=          prbsl4_file=
            prbsl5=          prbsl5_file=
            sect_got_plt=0   sect_got_plt_file=
            sect_hash=0      sect_gnu_file=
            sect_gnu_hash=0  sect_gnu_hash_file=
            for vv in k $VERS;do
                eval "${vv}lo=16777215 ${vv}lo_file="
                eval "${vv}lx=0        ${vv}lx_file="
                eval "${vv}hi=0        ${vv}hi_file="
            done
            for ff in $files;do
                cnt_f=`expr $cnt_f + 1`
                finfo=`file -b $ff`
                ftype=`echo "$finfo" | cut -d, -f1`
                echovv "ftype=$ftype ff=$ff"
                case "$ftype" in
                ELF*executable|ELF*shared\ object)
                    bits=`expr "$ftype" : 'ELF \([0-9]*\)-bit'`
                    echovv "bits=$bits"
                    ee=$finfo
                    if   kv=`expr "$ee" : '.*for [^ ]* \([0-9.]*\)'`;then
                        kv=`dot2dec $kv`
                        if [ $kv -lt $klo ];then klo=$kv klo_file=$ff;fi
                        if [ $kv -gt $khi ];then khi=$kv khi_file=$ff;fi
                        os=`expr "$ee" : '.*for \([^ ,]*\)'` os_file=$ff
                    elif os_=`expr "$ee" : '.*for \([^ ,]*\)'`;then
                        os=$os_  os_file=$ff
                    else
                        # old file (no "for ")
                        :
                    fi
                    os=`adjust_os "$os"`
                    elfVS=`readelf -VS $ff`
                    for vv in $VERS;do
                        vers=`echo "$elfVS" | awkvers $vv`
                        lo_hi=`echo "$vers" | awkhilo`
                        if lo=`expr "$lo_hi" : '\([0-9][0-9]*\)'`;then
                            hi=`expr "$lo_hi" : '[0-9][0-9]* \([0-9][0-9]*\)'`
                            eval "\
                            if [ $lo -lt \$${vv}lo ];then ${vv}lo=$lo ${vv}lo_file=$ff;fi
                            if [ $lo -gt \$${vv}lx ];then ${vv}lx=$lo ${vv}lx_file=$ff;fi
                            if [ $hi -gt \$${vv}hi ];then ${vv}hi=$hi ${vv}hi_file=$ff;fi
                            "
                        fi
                    done
                    if echo "$elfVS" | grep ' \.got\.plt' >/dev/null;then
                        sect_got_plt=1  sect_got_plt_file=$ff
                    fi
                    if echo "$elfVS" | grep ' \.hash' >/dev/null;then
                        sect_hash=1     sect_hash_file=$ff
                    fi
                    if echo "$elfVS" | grep ' \.gnu\.hash' >/dev/null;then
                        sect_gnu_hash=1 sect_gnu_hash_file=$ff
                    fi
                    ;;
                "current ar archive"|ELF*relocatable)
                    # attempt to look inside the archive for ELF files
                    ee=`readelf -hg $ff`
                    if cc=`echo "$ee" | grep 'Class:.*ELF'`;then
                        bits=`expr "$cc" : '.*ELF\([0-9]*\)'`
                        echovv "bits=$bits"
                        if echo "$ee" | grep 'i686\.get_pc_' >/dev/null;then
                            if [ -z "$prbsl5" ];then prbsl5=Y prbsl5_file=$ff;fi
                        else
                            if [ -z "$prbsl4" ];then prbsl4=Y prbsl4_file=$ff;fi
                        fi
                    fi
                    ;;
                esac
            done
            if [   $klo -eq 16777215 -a $khi -eq 0 \
                -a $GLIBC_lo -eq 16777215 -a $GLIBC_hi -eq 0 \
                -a -z "$prbsl4" -a -z "$prbsl5" ];then
                echov "for dir:$dd/$ss: NULL"
                if echo "$dd" | egrep '/lib$|/lib\.[^/]*$' >/dev/null;then
                    popd >/dev/null;continue
                fi
            elif true;then
                echov "for dir:$dd/$ss: os=$os  os_file=$os_file"
                for vv in k $VERS;do
                    eval "lo=\$${vv}lo lo_file=\$${vv}lo_file"
                    eval "lx=\$${vv}lx lx_file=\$${vv}lx_file"
                    eval "hi=\$${vv}hi hi_file=\$${vv}hi_file"
                    if [ $lo -lt 16777215 ];then
                        echov "     ${vv}lo=`dec2dot $lo` ${vv}lo_file=$lo_file"
                    fi
                    if [ $lx -gt 0 -a $lx -ne $lo ];then 
                        echov "     ${vv}lx=`dec2dot $lx` ${vv}lx_file=$lx_file"
                    fi
                    if [ $hi -gt 0 -a $hi -ne $lx ];then 
                        echov "     ${vv}hi=`dec2dot $hi` ${vv}hi_file=$hi_file"
                    fi
                done
                for vv in 4 5;do
                    eval prb=\$prbsl${vv}
                    if [ "$prb" ];then
                        eval "echov \"     prbsl$vv=$prb     prbsl${vv}_file=\$prbsl${vv}_file\""
                    fi
                done
            elif true;then
                echo "for dir:$dd/$ss: $os+2.6"
            elif true;then
                echo "for dir:$dd/$ss: $os+2.6-2.5"
            else
                echo "for dir:$dd/$ss: $os+$os_v2-$glib_v2"
            fi
            vers_str=$klo
            for vv in $VERS;do
                eval "hi=\$${vv}hi hi_file=\$${vv}hi_file"
                vers_str="$vers_str $hi"
            done
            add_to_fq_list "$fq" "$fq_dd" $os "$bits" $vers_str $sect_got_plt $sect_hash $sect_gnu_hash
        else
            echov "no files in subdir $ss"
        fi # "$files"
        popd >/dev/null
    done   # ss
    popd >/dev/null
done   # dd
list_sz=`echo "$fq_list" | wc -l`
echov "fq_list:"
echov "$fq_list"
if [ "$fq_list" ];then
    IFSsav=$IFS
    #IFS="${tab}"
    echo "$fq_list" | while read fq dd os bit klo GLIBC_hi GLIBCXX_hi CXXABI_hi sect_got_plt sect_hash sect_gnu_hash;do
        echov "-----> fq=$fq dd=$dd os=$os bit=$bit klo=$klo"
        kvdot=`dec2dot $klo`
        if [ "$bit" = 64 ];then bit=64bit; else bit=; fi

        # Marc says basically
        #   lts3 = ups flavor -1   ==>  Linux
        #   sl4  = ups flavor -3   ==>  Linux+2.6
        #   sl5  = ups flavor -4   ==>  Linux+2.6-2.5
        # for SL4: (ref notes for 2009.08.24)
        # SO, the "default" for 32 bit (i386/i686??) seems to be 2.0.0 while the
        # default for 64 bit seems to be 2.4.0
        if [ "$bit" = 64bit ];then
            case "$sect_got_plt $sect_hash $sect_gnu_hash" in
            "0 1 0") echo "flavor=$os$bit fq=$fq";;   # LTS3
            "1 1 0")                                  # SLF4
                if [ $klo -eq 16777215 ];then kvdot=2.6; fi
                if [ "${opt_ups-}" ];then kvdot=2.6; fi
                flavor=$os$bit+`expr $kvdot : '\([0-9]*\.[0-9]*\)'`
                echo "flavor=$flavor fq=$fq";;
            "1 0 1")                                  # SLF5
                if [ "${opt_ups-}" ];then
                    gldot=2.5 kvdot=2.6
                else
                    if [ $klo -eq 16777215 ];then kvdot=2.6; fi
                    gldot=`dec2dot $GLIBC_hi`
                fi
                flavor=$os$bit+`expr $kvdot : '\([0-9]*\.[0-9]*\)'`-`expr $gldot : '\([0-9]*\.[0-9]*\)'`
                echo "flavor=$flavor fq=$fq";;
            *)
                if [ $klo -eq 16777215 -a $os = ANY ];then
                    if [ "$fq" != . -o $list_sz -eq 1 ];then
                        echo "flavor=NULL fq=$fq"
                    fi
                elif [   \( $klo      -ge 132608 -a $klo      -lt 16777215 \) \
                    -o \( $GLIBC_hi -ge 132096 -a $GLIBC_hi -lt 16777215 \) ];then
                    # could be SL5
                    if [ $klo -eq 16777215 ];then kvdot=2.6; fi
                    if [ "${opt_ups-}" ];then gldot=2.5
                    else                      gldot=`dec2dot $GLIBC_hi`; fi
                    flavor=$os$bit+`expr $kvdot : '\([0-9]*\.[0-9]*\)'`-`expr $gldot : '\([0-9]*\.[0-9]*\)'`
                    echo "flavor=$flavor fq=$fq"
                elif [   \( $klo      -ge 131589 -a $klo      -lt   132617 \) \
                    -o \( $GLIBC_hi -ge 131840 -a $GLIBC_hi -lt   132096 \) ];then
                    # could be SL4
                    if [ $klo -eq 16777215 ];then kvdot=2.6; fi
                    if [ "${opt_ups-}" ];then kvdot=2.6; fi
                    flavor=$os$bit+`expr $kvdot : '\([0-9]*\.[0-9]*\)'`
                    echo "flavor=$flavor fq=$fq"
                elif [ $klo -ge 131072 -a $klo -lt   131589 ];then  # 2.0.0
                    # could be LTS3
                    flavor=$os$bit+`expr $kvdot : '\([0-9]*\.[0-9]*\)'`
                    if [ "${opt_ups-}" ];then flavor=$os$bit; fi
                    echo "flavor=$flavor fq=$fq"
                else
                    echo "flavor=$os fq=$fq"
                fi
            esac
        else
            case "$sect_got_plt $sect_hash $sect_gnu_hash" in
            "0 1 0") echo "flavor=$os$bit fq=$fq";;   # LTS3
            "1 1 0")                                  # SLF4
                if [ $klo -eq 16777215 ];then kvdot=2.6; fi
                if [ "${opt_ups-}" ];then kvdot=2.6; fi
                flavor=$os$bit+`expr $kvdot : '\([0-9]*\.[0-9]*\)'`
                echo "flavor=$flavor fq=$fq";;
            "1 0 1")                                  # SLF5
                if [ "${opt_ups-}" ];then
                    gldot=2.5 kvdot=2.6
                else
                    if [ $klo -eq 16777215 ];then kvdot=2.6; fi
                    gldot=`dec2dot $GLIBC_hi`
                fi
                flavor=$os$bit+`expr $kvdot : '\([0-9]*\.[0-9]*\)'`-`expr $gldot : '\([0-9]*\.[0-9]*\)'`
                echo "flavor=$flavor fq=$fq";;
            *)
                if [ $klo -eq 16777215 -a $os = ANY ];then
                    if [ "$fq" != . -o $list_sz -eq 1 ];then
                        echo "flavor=NULL fq=$fq"
                    fi
                elif [   \( $klo      -ge 132608 -a $klo      -lt 16777215 \) \
                    -o \( $GLIBC_hi -ge 132096 -a $GLIBC_hi -lt 16777215 \) ];then
                    # could be SL5
                    if [ $klo -eq 16777215 ];then kvdot=2.6; fi
                    if [ "${opt_ups-}" ];then gldot=2.5
                    else                      gldot=`dec2dot $GLIBC_hi`; fi
                    flavor=$os$bit+`expr $kvdot : '\([0-9]*\.[0-9]*\)'`-`expr $gldot : '\([0-9]*\.[0-9]*\)'`
                    echo "flavor=$flavor fq=$fq"
                elif [   \( $klo      -ge 131072 -a $klo      -lt   132608 \) \
                    -o \( $GLIBC_hi -ge 131840 -a $GLIBC_hi -lt   132096 \) ];then
                    # could be SL4
                    if [ $klo -eq 16777215 ];then kvdot=2.6; fi
                    if [ "${opt_ups-}" ];then kvdot=2.6; fi
                    flavor=$os$bit+`expr $kvdot : '\([0-9]*\.[0-9]*\)'`
                    echo "flavor=$flavor fq=$fq"
                else
                    # could be LTS3
                    echo "flavor=$os fq=$fq"
                fi
            esac
        fi
    done
    IFS=$IFSsav
fi

if [ $cnt_f -gt 0 ];then echov cnt_f=$cnt_f;fi
