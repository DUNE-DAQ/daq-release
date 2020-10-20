#! /bin/sh
#  This file (make_tar.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
#  Nov 25, 2009. "TERMS AND CONDITIONS" governing this file are in the README
#  or COPYING file. If you do not have such a file, one can be obtained by
#  contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#  $RCSfile: make_tar.sh,v $
#  rev='$Revision: 1.2 $$Date: 2009/11/25 23:07:47 $'
set -u
if [ "${1-}" = -x ];then set -x; shift;fi

PROD_NAME=ups
USAGE="\
  usage: `basename $0` <release>
example: `basename $0` v4_7

Note: This script will create a $PROD_NAME release gzipped
      tar achive with the extension .tgz (for example, $PROD_NAME-1.0.tgz)
      in the current working directory. The top level src directory is assumed
      to be the parent directory of the location of this script. (i.e.
      this script will be execute while it is located in .../bin)
      The release specification must
        - begin with a letter followed by a number
        - end with a number or letter
        - contain only numbers, letters and '_'
        - not contain 2 or more consequtive '_'
      The directory \".../$PROD_NAME/<release>\" must not exist.
      The tar file $PROD_NAME-<release>.tgz must not exist.
Note: The multi-level directory structure that is contained in the
      tar file that this script produces in intended to allow for the
      unwinding into a ups product directory structure that supports
      multiple versions and multiple flavor for each of those version without
      src duplication.
Note: The tar specification does not exclude all built files (man pages, for
      example).
"

if expr "x${1-}" : 'x-' >/dev/null || [ $# -ne 1 ];then
    echo "$USAGE"; exit 1
fi

# must begin with a number and end with either a number or letter
if       expr "$1" :   '[a-zA-Z][0-9]' >/dev/null \
  && rr=`expr "$1" : '\([0-9a-zA-Z_]*[0-9a-zA-Z]$\)'`;then
    if expr "$1" : '.*__' >/dev/null;then
        echo invalid release specification
        echo "$USAGE"; exit 1
    fi
    RELEASE=$rr
else
    echo invalid release specification
    echo "$USAGE"; exit 1
fi

#  done with arg parsing
# - - - - - - - - - - - - -

appdir=`dirname $0`    # assuming this to be src_root/bin
src_root=`CDPATH= cd "$appdir/..";pwd`

src_root_base=`basename "$src_root"`
src_root_parent=`dirname "$src_root"`

if ls -ld $PROD_NAME-$RELEASE.tgz 2>/dev/null;then
    echo "File exists; move it out of the way or delete it."
    echo "$USAGE"; exit 1
fi

put_back()
{
    if [ -d "$src_root_parent/$PROD_NAME/$RELEASE" -a ! -d "$src_root" ];then
        mv "$src_root_parent/$PROD_NAME/$RELEASE" "$src_root"
        if [ "${rmdir_prod_name-}" ];then rmdir $src_root_parent/$PROD_NAME; fi
        echo "temporary rename undone." 
    fi
}

echo "cd \"$src_root_parent\""
cd "$src_root_parent"

if [ ! -d $PROD_NAME ];then mkdir $PROD_NAME; rmdir_prod_name=1
else                        echo directory $PROD_NAME exists - OK; fi
if ls -d $PROD_NAME/$RELEASE >/dev/null 2>&1;then
    echo "Error - directory entry $PROD_NAME/$RELEASE exists. Please"
    echo "        move it out of the way or choose another release spec."
    exit 1
else
    echo "temporarily renaming -"
    echo "   $src_root"
    echo "to $PROD_NAME/$RELEASE"
    trap put_back 0
    mv $src_root $PROD_NAME/$RELEASE
fi

arg_dir="\"$PROD_NAME-$RELEASE.tgz\" \"$PROD_NAME/$RELEASE\""
tar_cmd="tar cz --wildcards --exclude '*~'\
 --exclude '*.o'\
 -f $arg_dir/Makefile\
 \"$PROD_NAME/$RELEASE/bin\"\
 \"$PROD_NAME/$RELEASE/bugs\"\
 \"$PROD_NAME/$RELEASE/doc\"\
 \"$PROD_NAME/$RELEASE/inc\"\
 \"$PROD_NAME/$RELEASE/init.d\"\
 \"$PROD_NAME/$RELEASE/lib\"\
 \"$PROD_NAME/$RELEASE/man\"\
 \"$PROD_NAME/$RELEASE/src\"\
 \"$PROD_NAME/$RELEASE/test\"\
 \"$PROD_NAME/$RELEASE/ups\"\
"
if [ -d "$PROD_NAME/$RELEASE/CVS" ];then
    tar_cmd="$tar_cmd \"$PROD_NAME/$RELEASE/CVS\""
fi

echo "executing: $tar_cmd"
eval "$tar_cmd"

echo "executing: mv $arg_dir"
eval "mv $arg_dir"

#cleanup on exit ("signal" 0)
