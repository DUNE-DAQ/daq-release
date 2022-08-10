from spack import *
from spack.util.environment import is_system_path
import sys
import os

class Uhal(Package):

    homepage = "https://github.com/ipbus/"
    url      = "https://codeload.github.com/ipbus/ipbus-software/tar.gz/refs/tags/v2.8.1"

    version('2.8.1', sha256='73f26639a16ea65cecd367045ad0767a7eb4f5f8f488df1bbf121fc47aec3142', extension='tar.gz')

    #depends_on('boost@1.75.0+debug', type='build')
    depends_on('boost', type='build')
    depends_on('pugixml', type=('build', 'link', 'run'))
    depends_on('gettext', type=('build', 'link', 'run'))
    depends_on('py-pybind11', type=('build', 'link', 'run'))

    #patch('ipbus_2.patch', when='@2.8.0')
    patch('ipbus_2.patch', when='@2.8.1')

    def setup_build_environment(self,env):
        spec=self.spec
        env.set('Set','uhal')


        def add_include_path(dep_name):
            include_path = spec[dep_name].prefix.include
            if not is_system_path(include_path):
                env.append_path('SPACK_INCLUDE_DIRS', include_path)

        # With that done, let's go fixing those deps
        add_include_path('boost')
        add_include_path('pugixml')

        env.set('EXTERN_BOOST_INCLUDE_PREFIX', spec['boost'].prefix.include.boost)
        env.set('EXTERN_BOOST_LIB_PREFIX', spec['boost'].prefix.lib)
        env.set('EXTERN_PUGIXML_INCLUDE_PREFIX', spec['pugixml'].prefix.include)
        env.set('EXTERN_PUGIXML_LIB_PREFIX', spec['pugixml'].prefix.lib64)

    def setup_run_environment(self, env):
        env.prepend_path('PYTHONPATH', os.path.join(self.prefix.lib, "python3.8/site-packages"))

    def patch(self):
        copy(join_path(os.path.dirname(__file__),
             "uhalConfig.cmake"), "uhalConfig.cmake")
        copy(join_path(os.path.dirname(__file__),
             "uhalConfigVersion.cmake"), "uhalConfigVersion.cmake")
        copy(join_path(os.path.dirname(__file__),
             "uhalTargets.cmake"), "uhalTargets.cmake")

    def install(self, spec, prefix):
        dest=prefix
        make()
#        make('install')
        make('prefix=' + dest, 'install')
        install('uhalConfig.cmake',prefix)
        install('uhalConfigVersion.cmake',prefix)
        install('uhalTargets.cmake',prefix)

