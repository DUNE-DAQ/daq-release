
# JCF, May-28-2024

# This script is designed specifically to address daq-release Issue 370,
# i.e., the renaming of a couple of our OKS packages. Please note that
# once the renaming is complete and the Issue closed this script can be
# deleted from this repo

scriptdir=$(dirname "$(realpath "${BASH_SOURCE[0]}")")


for pkg in oksdbinterfaces oksconfig genconfig ; do
    git clone https://github.com/DUNE-DAQ/$pkg -b develop
done

##################### oksdbinterfaces -> conffwk #######################

cd oksdbinterfaces 

git mv ./include/oksdbinterfaces ./include/conffwk
git mv ./python/oksdbinterfaces ./python/conffwk
git mv ./cmake/oksdbinterfacesConfig.cmake.in ./cmake/conffwkConfig.cmake.in

git_stashing_area=$(mktemp -d)
mv .git $git_stashing_area

find . -type f | xargs -i sed -r -i 's/oksdbinterfaces/conffwk/g' {}

# OKSDB_INTERFACE is not a typo
find . -type f | xargs -i sed -r -i 's/OKSDB_INTERFACE/CONFFWK/g' {}

mv $git_stashing_area/.git .
rm -rf $git_stashing_area

cd ..
mv oksdbinterfaces conffwk

##################### oksconfig -> oksconflibs #############################

cd oksconfig

git mv ./include/oksconfig ./include/oksconflibs
git mv ./cmake/oksconfigConfig.cmake.in ./cmake/oksconflibsConfig.cmake.in

git_stashing_area=$(mktemp -d)
mv .git $git_stashing_area

# Note that oksconflibs depends on conffwk, so we need to account for the update
find . -type f | xargs -i sed -r -i 's/oksdbinterfaces/conffwk/g' {}

find . -type f | xargs -i sed -r -i 's/oksconfig/oksconflibs/g' {}
find . -type f | xargs -i sed -r -i 's/OKSCONFIG/OKSCONFLIBS/g' {}

mv $git_stashing_area/.git .
rm -rf $git_stashing_area

cd ..
mv oksconfig oksconflibs

##################### genconfig -> oksdalgen #############################

cd genconfig 

git mv ./cmake/genconfigConfig.cmake.in ./cmake/oksdalgenConfig.cmake.in

git_stashing_area=$(mktemp -d)
mv .git $git_stashing_area

# Note that oksdalgen depends on conffwk, so we need to account for the update   
find . -type f | xargs -i sed -r -i 's/oksdbinterfaces/conffwk/g' {}

find . -type f | xargs -i sed -r -i 's/genconfig/oksdalgen/g' {}
find . -type f | xargs -i sed -r -i 's/GENCONFIG/OKSDALGEN/g' {}

git mv apps/genconfig.cxx apps/oksdalgen.cxx

mv $git_stashing_area/.git .
rm -rf $git_stashing_area

cd ..
mv genconfig oksdalgen

###########################Change dependencies with other packages#############################

# Add docs (official documentation) and dal (useful standalone
# tutorial package) to the regular list of v5 packages to modify

for pkg in docs dal $( $scriptdir/list_packages.py develop coredaq ) $( $scriptdir/list_packages.py develop fddaq ) ; do
    
    if [[ $pkg == "oksdbinterfaces" || $pkg == "genconfig" || $pkg == "oksconfig" ]]; then
	continue
    fi
	
    git clone http://github.com/DUNE-DAQ/$pkg -b develop

    cd $pkg || return "Couldn't clone $pkg"

    git_stashing_area=$(mktemp -d)
    mv .git $git_stashing_area

    find . -type f | xargs -i sed -r -i 's/oksdbinterfaces/conffwk/g' {}
    find . -type f | xargs -i sed -r -i 's/genconfig/oksdalgen/g' {}
    find . -type f | xargs -i sed -r -i 's/oksconfig/oksconflibs/g' {}
    find . -type f | xargs -i sed -r -i 's/GENCONFIG/OKSDALGEN/g' {}  # Needed for daq-cmake
    
    mv $git_stashing_area/.git .
    rm -rf $git_stashing_area

    echo $pkg
    git diff HEAD --name-status
	
    if [[ -z $( git diff HEAD --name-status ) ]]; then
	cd ..
	rm -rf $pkg
    else
	cd ..
    fi
    
done


