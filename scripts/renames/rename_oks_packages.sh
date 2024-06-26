
# JCF, May-28-2024

# This script is designed specifically to address daq-release Issue 370,
# i.e., the renaming of a couple of our OKS packages. Please note that
# once the renaming is complete and the Issue closed this script can be
# deleted from this repo

# JCF, Jun-6-2024

# Overhaul this script to handle daq-release Issue 373, concerning
# further renaming

scriptdir=$(dirname "$(realpath "${BASH_SOURCE[0]}")")


for pkg in coredal appdal ; do
    git clone https://github.com/DUNE-DAQ/$pkg -b develop
done

##################### coredal -> confmodel #######################

cd coredal

git mv ./include/coredal ./include/confmodel
git mv ./python/coredal ./python/confmodel
git mv ./schema/coredal ./schema/confmodel
git mv ./cmake/coredalConfig.cmake.in ./cmake/confmodelConfig.cmake.in

git_stashing_area=$(mktemp -d)
mv .git $git_stashing_area

find . -type f | xargs -i sed -r -i 's/coredal/confmodel/g' {}

mv $git_stashing_area/.git .
rm -rf $git_stashing_area

cd ..
mv coredal confmodel

##################### appdal -> appmodel #############################

cd appdal

git mv ./config/appdal ./config/appmodel
git mv ./include/appdal ./include/appmodel
git mv ./python/appdal ./python/appmodel
git mv ./schema/appdal ./schema/appmodel

git mv ./cmake/appdalConfig.cmake.in ./cmake/appmodelConfig.cmake.in
git mv ./include/appmodel/appdalIssues.hpp ./include/appmodel/appmodelIssues.hpp

git_stashing_area=$(mktemp -d)
mv .git $git_stashing_area

# Note that appmodel depends on confmodel, so we need to account for the update
find . -type f | xargs -i sed -r -i 's/coredal/confmodel/g' {}

find . -type f | xargs -i sed -r -i 's/appdal/appmodel/g' {}
find . -type f | xargs -i sed -r -i 's/APPDAL/APPMODEL/g' {}
find . -type f | xargs -i sed -r -i 's/Appdal/Appmodel/g' {}

mv $git_stashing_area/.git .
rm -rf $git_stashing_area

cd ..
mv appdal appmodel

###########################Change dependencies with other packages#############################

# Add docs (official documentation) and dal (useful standalone
# tutorial package) to the regular list of v5 packages to modify

# JCF, May-30-2024: Also add drunc and integrationtest

for pkg in drunc integrationtest docs dal $( $scriptdir/list_packages.py develop coredaq ) $( $scriptdir/list_packages.py develop fddaq ) ; do
    
    if [[ $pkg == "coredal" || $pkg == "appdal" ]]; then
	continue
    fi
	
    git clone http://github.com/DUNE-DAQ/$pkg -b develop

    cd $pkg || return "Couldn't clone $pkg"

    git_stashing_area=$(mktemp -d)
    mv .git $git_stashing_area

    find . -type f | xargs -i sed -r -i 's/coredal/confmodel/g' {}
    find . -type f | xargs -i sed -r -i 's/appdal/appmodel/g' {}
    
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


