#/bin/bash

if (( $# < 1 || $# > 2 )) ; then
    echo "Usage: "$(basename $0)" <spack package - e.g., dune-daqpackages@develop>"
    exit 1
fi

pkg=$1

tmpfile=$(mktemp)
for dirname in $( spack find -d -p $pkg | sed -r -n '/'$pkg'/,$s/.*\s+(\S+)\s*$/\1/p' ); do
    du $dirname -s
done | tee $tmpfile

num_packages=$( wc -l $tmpfile | awk '{print $1}' )
echo
echo "$num_packages packages total"
cat $tmpfile | awk '{tot += $1} END {printf("%d bytes total\n", tot)}'

if [[ -e $tmpfile ]]; then
    rm -f $tmpfile
fi
