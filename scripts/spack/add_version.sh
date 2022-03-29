#!/bin/bash

if [[ "$#" != "2" ]]; then
    echo "Usage: "$(basename $0)" <version> <package>" >&2
    exit 1
fi

version=$1
package=$2

packagefile=$PWD/dune_daqpackages/packages/$package/package.py

if ! [[ -e $packagefile ]]; then
    echo "Unable to locate $packagefile; exiting..." >&2
    exit 2
fi

url_replacement="https://codeload.github.com/DUNE-DAQ/$package/legacy.tar.gz/$version"
emptydir=$( mktemp -d )
cd $emptydir
curl --fail -O $url_replacement > /dev/null 2>&1 

if [[ "$?" != "0" ]]; then
    echo "Unable to locate $url_replacement; exiting..." >&2
    exit 3
fi

cmakelists_file=$( gtar ztvf $version | sed -r -n 's!.*\s+(.*/CMakeLists.txt).*!\1!p'  )

if [[ -z $cmakelists_file ]]; then
    echo "Unable to locate CMakeLists.txt file in $PWD/$version; exiting..." >&2
    exit 4
fi

gtar zxvf $version $cmakelists_file
pkg_version=$( sed -r -n 's/^\s*project\(.*\s+([0-9\.]+)\s*\).*/\1/p' $cmakelists_file )

if [[ -z $pkg_version ]]; then
    echo "Unable to determine package version from $cmakelists_file; exiting..." >&2
    exit 5
fi

echo "Package version in frozen release $version is $pkg_version"

sed -r -i 's!^(\s*)url(\s*)=(\s*).*!\1url\2=\3"'$url_replacement'"!' $packagefile
sed -r -i 's!^(\s*)version\("develop".*!\1version("develop", branch="develop", git="https://github.com/DUNE-DAQ/'$package'")!' $packagefile

spack checksum $package 
url_replacement="https://github.com/DUNE-DAQ/$package"
sed -r -i 's!^(\s*)url(\s*)=(\s*).*!\1url\2=\3"'$url_replacement'"!' $packagefile

sed -r -i '/^(\s*)version\("develop".*/a\    version("'$pkg_version'", sha256="FILL THIS IN", extension="tar.gz", url="https://codeload.github.com/DUNE-DAQ/'$package'/legacy.tar.gz/'$version'")' $packagefile

cat<<EOF

In order to add the version of $package corresponding to the frozen
release $version, ignore everything on the line above relevant to your
frozen release which starts with "    version(" besides the
checksum. Replace the phrase "FILL THIS IN" in $package's package.py
file with this checksum.

EOF
