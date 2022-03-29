
name=$1
version=$2
origdir=$PWD

echo "Will attempt to create a package.py for the $version version of the $name package"

cd /tmp
url=https://codeload.github.com/DUNE-DAQ/$name/legacy.tar.gz/$version
curl -O $url
gtar ztvf $PWD/$version >/dev/null 2>&1

if [[ "$?" != "0" ]]; then
    echo "Either $url doesn't exist or it's not a tarball; returning..." >&2
    cd $origdir
    return 1
fi

cd $origdir
spack create --skip-editor $url

if [[ "$?" != 0 ]]; then
    echo "There was a problem trying to create a package off of ${url}, returning..." >&2
    cd $origdir
    return 3
fi

packagefile=dune_daqpackages/packages/$name/package.py

if ! [[ -e $packagefile ]]; then
    echo "The \"spack create $url\" command should have created $packagefile but didn't; returning..." >&2
    cd $origdir
    return 2
fi

if [[ ! -e ${packagefile}.backup ]]; then
    cp -p $packagefile{,.backup}
fi

tmpdir=$(mktemp -d)
cd $tmpdir 
curl -O https://raw.githubusercontent.com/DUNE-DAQ/daq-release/develop/configs/$version/dbt-settings.sh
pkgversion=$( sed -r -n 's/_/./g;s/.*"'$name'\s+v([0-9.]+).*/\1/Ip' dbt-settings.sh )
cd -
if [[ "$tmpdir" =~ ^/tmp ]]; then
    rm -rf $tmpdir
fi

# Get rid of the boilerplate between the "-"'s in the header comment
sed -i -r '/^# -/,/^# -/d' $packagefile

# Get rid of everything from the "depends_on-foo" example onwards
sed -i -r '/.*depends_on.*/,$d' $packagefile

# Set the homepage to the one on our official website (we assume it exists)
sed -i -r 's!homepage = ".*"!homepage = "https://dune-daq-sw.readthedocs.io/en/'$version'/packages/'$name'/"!' $packagefile

# Yours truly will maintain the Spack package
sed -i -r 's!^(\s*)#\s*maintainers.*!\1maintainers = ["jcfreeman2"]!' $packagefile

# Switch the auto-generated version from the suite release version to the per-package version
sed -i -r 's/version\([^,]+,/version\("'$pkgversion'",/' $packagefile

# And since the new version probably means Spack can't auto-deduce the tarball, tell it explicitly what it is 
sed -i -r '/^\s*version/s!\)s*$!, extension="tar.gz", url="'$url'"\)!' $packagefile

echo "Please make your edits (e.g., add dependencies, remove FIXMEs, etc) by running \"spack edit $name\""

cat<<EF >> $packagefile

    depends_on("daq-cmake")
    depends_on('py-moo', type='build')

    def setup_run_environment(self, env):
        env.set(self.__module__.split(".")[-1].upper().replace("-", "_") + "_SHARE", self.prefix + "/share" )
        env.prepend_path('DUNEDAQ_SHARE_PATH', self.prefix + "/share")
        env.prepend_path('CET_PLUGIN_PATH', self.prefix.lib + "64")
        env.prepend_path("PYTHONPATH", self.prefix.lib + "64/python")


EF

return 0
