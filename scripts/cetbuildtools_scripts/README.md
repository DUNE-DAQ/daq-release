# UPS product implementation notes

The build scripts in this directory create UPS products for the DUNE DAQ application framework external dependencies. The general strategy is to use the template CMake scripts from
[`cetbuildtools`](https://cdcvs.fnal.gov/redmine/projects/cetbuildtools/wiki) to get the necessary UPS variables and files. The package itself is built as a "sub-project" using CMake's 
[`ExternalProject`](https://cmake.org/cmake/help/latest/module/ExternalProject.html) functionality. (It's possible, and in fact likely, that this would all be better achieved with `FetchContent`, but I didn't try)

There are some quirks:

The `cetbuildtools` use `CPack` to create a tarball of the install directory, which we can untar in a ups products directory elsewhere. `CPack` appears to only know about files that are `install()`ed with CMake, so we can't just point `ExternalProject` directly at `${CMAKE_INSTALL_PREFIX}`. Instead, we have to point the `ExternalProject` install at a temporary staging area, and manually `install()` the directories we want, making sure to install into relative paths, so that the final build is relocatable.

`cetbuildtools` has its own function (`cet_cmake_config()`) to create a CMake [package config file](https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#package-configuration-file), but this config file doesn't know about the targets exported by the package itself (ie, the `Foo::foo`-style targets that handle transitive dependencies nicely), so we don't call `cet_cmake_config()`, and instead we just use the config file provided by the package itself. Happily, when the resulting UPS product is `setup`, `CMAKE_PREFIX_PATH` is set appropriately, and the package config file can be found by `find_package`.
