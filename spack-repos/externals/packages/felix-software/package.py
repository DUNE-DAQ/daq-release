from spack import *
import os
import sys

from shutil import copytree
import spack.util.environment as envutil

class FelixSoftware(Package):

    git = "https://github.com/jcfreeman2/intentionallyempty.git"

    version("dunedaq-v2.10.0")
    version("dunedaq-v2.8.0")

    depends_on('boost', type='build')
    depends_on('python', type='build')
    depends_on('cmake', type='build')
#    depends_on('qt@5.15.2', type='build')
#    depends_on('intel-tbb', type='build')
    depends_on('yaml-cpp', type='build')
    depends_on('czmq', type='build')
    depends_on('cppzmq', type='build')
    depends_on('py-pybind11', type=('build', 'link', 'run'))
    depends_on("nlohmann-json")

    def setup_build_environment(self, env):
        if not "REGMAP_VERSION" in os.environ:
            env.set("REGMAP_VERSION", "0x0500")


    def install(self, spec, prefix):

        hashes = {}
        if self.spec.satisfies('@dunedaq-v2.10.0'):
            hashes["cmake_tdaq"] = "d66ce21b"
            hashes["drivers_rcc"] = "c5c21e00"
            hashes["flxcard"] = "7614b8cc"
            hashes["regmap"] = "f0fd698e"
            hashes["packetformat"] = "a6933e36"
            hashes["flxcard_py"] = "61001bd6"
            hashes["ftools"] = "c326e788"
            hashes["external-catch"] = "6a9aa08a"
            
            felix_version="dunedaq-v2.10.0"

        elif self.spec.satisfies('@dunedaq-v2.8.0'):
            hashes["cmake_tdaq"] = "d66ce21b"
            hashes["drivers_rcc"] = "b37bd757"
            hashes["flxcard"] = "683d9696"
            hashes["regmap"] = "87ce47ba"
            hashes["packetformat"] = "a84931eb"
            hashes["flxcard_py"] = "61001bd6"
            hashes["ftools"] = "1cfd1b56"
            hashes["external-catch"] = "6a9aa08a"

            felix_version="dunedaq-v2.8.0"

        copytree('.', prefix.software)

        copy(join_path(os.path.dirname(__file__),
                       f"felixConfig.cmake.{felix_version}"), self.prefix + "/felixConfig.cmake")
        copy(join_path(os.path.dirname(__file__),
                       f"felixConfigVersion.cmake.{felix_version}"), self.prefix + "/felixConfigVersion.cmake")
        copy(join_path(os.path.dirname(__file__),
                       f"felixTargets.cmake.{felix_version}"), self.prefix + "/felixTargets.cmake")

        def return_zero_or_exit(cmd):
            retval = os.system(cmd)
            if retval != 0:
                sys.stderr.write(f"Error: return value of {retval} for the command \"{cmd}\"")
                sys.exit(retval)

        with working_dir(prefix.software):

            install(f"{os.path.dirname(__file__)}/CMakeLists_base_of_software_repo.txt", "CMakeLists.txt")

            print("About to clone https://gitlab.cern.ch/atlas-tdaq-felix/cmake_tdaq.git")
            return_zero_or_exit('git clone https://gitlab.cern.ch/atlas-tdaq-felix/cmake_tdaq.git')
            return_zero_or_exit('pushd cmake_tdaq && git checkout %s && popd' % (hashes["cmake_tdaq"]))
            return_zero_or_exit('sed -i \'2 i set(NOLCG TRUE)\' cmake_tdaq/cmake/modules/FELIX.cmake')
 
            print("About to clone https://gitlab.cern.ch/atlas-tdaq-felix/drivers_rcc.git")
            return_zero_or_exit('git clone https://gitlab.cern.ch/atlas-tdaq-felix/drivers_rcc.git')
            return_zero_or_exit('pushd drivers_rcc && git checkout %s && popd' % (hashes["drivers_rcc"]))

            print("About to clone https://gitlab.cern.ch/atlas-tdaq-felix/flxcard.git")
            return_zero_or_exit('git clone https://gitlab.cern.ch/atlas-tdaq-felix/flxcard.git')
            return_zero_or_exit('pushd flxcard && git checkout %s && popd' % (hashes["flxcard"]))

            print("About to clone https://gitlab.cern.ch/atlas-tdaq-felix/regmap.git")
            return_zero_or_exit('git clone https://gitlab.cern.ch/atlas-tdaq-felix/regmap.git')
            return_zero_or_exit('pushd regmap && git checkout %s && popd' % (hashes["regmap"]))

            print("About to clone https://gitlab.cern.ch/atlas-tdaq-felix/packetformat.git")
            return_zero_or_exit('git clone https://gitlab.cern.ch/atlas-tdaq-felix/packetformat.git')
            return_zero_or_exit('pushd packetformat && git checkout %s && popd' % (hashes["packetformat"]))

            print("About to clone https://gitlab.cern.ch/atlas-tdaq-felix/flxcard_py.git")
            return_zero_or_exit('git clone https://gitlab.cern.ch/atlas-tdaq-felix/flxcard_py.git')
            return_zero_or_exit('pushd flxcard_py && git checkout %s && popd' % (hashes["flxcard_py"]))
            install(f"{os.path.dirname(__file__)}/flxcard_py_CMakeLists.txt.{felix_version}", prefix+"/software/flxcard_py/CMakeLists.txt")

            print("Aout to clone https://gitlab.cern.ch/atlas-tdaq-felix/ftools.git")
            return_zero_or_exit('git clone https://gitlab.cern.ch/atlas-tdaq-felix/ftools.git')
            return_zero_or_exit('pushd ftools && git checkout %s && popd' % (hashes["ftools"]))
            install(f"{os.path.dirname(__file__)}/ftools_CMakeLists.txt.{felix_version}", prefix+"/software/ftools/CMakeLists.txt")

            return_zero_or_exit('git clone https://gitlab.cern.ch/atlas-tdaq-felix/external-catch.git external/catch')
            return_zero_or_exit('pushd external/catch && git checkout %s && popd' % (hashes["external-catch"]))

            os.environ['PATH'] = prefix+"/software/cmake_tdaq/bin" + ":" + os.environ['PATH']
            return_zero_or_exit('cmake_config x86_64-centos7-gcc8-opt') 
            os.chdir("x86_64-centos7-gcc8-opt")
            make()

        # JCF, Oct-21-2021
        # Perform the same copies as in daq-release's build_felix.sh excepting ftools which may have build issues

        with working_dir(prefix):
        
            for subdir in ["include", "lib", "bin"]:
                os.mkdir(subdir)

            copytree("software/drivers_rcc", "drivers_rcc")
            copytree("software/regmap/regmap", "include/regmap")
            copytree("software/drivers_rcc/cmem_rcc", "include/cmem_rcc")
            copytree("software/drivers_rcc/rcc_error", "include/rcc_error")
            copytree("software/flxcard/flxcard", "include/flxcard")
            copytree("software/packetformat/packetformat", "include/packetformat")

            return_zero_or_exit("cp software/drivers_rcc/lib64/lib* lib")
            return_zero_or_exit("cp software/x86_64-centos7-gcc8-opt/flxcard/lib* lib")
            return_zero_or_exit("cp software/x86_64-centos7-gcc8-opt/flxcard_py/lib* lib")
            return_zero_or_exit("cp software/x86_64-centos7-gcc8-opt/packetformat/lib* lib")
            return_zero_or_exit("cp software/x86_64-centos7-gcc8-opt/regmap/lib* lib")
            return_zero_or_exit("cp software/x86_64-centos7-gcc8-opt/drivers_rcc/lib* lib")
            return_zero_or_exit("cp software/x86_64-centos7-gcc8-opt/ftools/libFlxTools* lib")

            return_zero_or_exit("cp software/x86_64-centos7-gcc8-opt/flxcard/flx-* bin")
            return_zero_or_exit("cp software/x86_64-centos7-gcc8-opt/ftools/f* bin")

            return_zero_or_exit("rm -rf software")
