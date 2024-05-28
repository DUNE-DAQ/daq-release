
# JCF, May-28-2024

# This script is designed specifically to address daq-release Issue 370,
# i.e., the renaming of a couple of our OKS packages. Please note that
# once the renaming is complete and the Issue closed this script can be
# deleted from this repo

##################### oksdbinterfaces -> conffwk #######################

cd oksdbinterfaces || return "No oksdbinterfaces repo found"

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

cd oksconfig || return "No oksconfig repo found"

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

cd genconfig || return "No genconfig repo found"

git mv ./cmake/genconfigConfig.cmake.in ./cmake/oksdalgenConfig.cmake.in

git_stashing_area=$(mktemp -d)
mv .git $git_stashing_area

# Note that oksdalgen depends on conffwk, so we need to account for the update   
find . -type f | xargs -i sed -r -i 's/oksdbinterfaces/conffwk/g' {}

find . -type f | xargs -i sed -r -i 's/genconfig/oksdalgen/g' {}
find . -type f | xargs -i sed -r -i 's/GENCONFIG/OKSDALGEN/g' {}

# For the time being retain the name of the executable despite the package name changing
sed -r -i 's/oksdalgen oksdalgen.cxx/genconfig genconfig.cxx/' CMakeLists.txt

mv $git_stashing_area/.git .
rm -rf $git_stashing_area

cd ..
mv genconfig oksdalgen
