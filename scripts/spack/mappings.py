cmake_to_spack = {
    'py-moo': 'py-moo\", type=\"build',
    'pybind11': 'py-pybind11',
    'folly': 'folly cxxstd=2a',
    'libtorrentrasterbar': 'libtorrent',
    'felix': 'felix-software',
    'nlohmann_json': 'nlohmann-json',
    'absl': 'abseil-cpp',
    'msgpack': 'msgpack-c',
    'rdkafka': 'librdkafka',
    'qt5': 'qt',
    'highfive': 'highfive +mpi',
    'pkgconfig': 'pkgconf',
    'tbb': 'intel-tbb',
}

pyvenv_url_names = {
    "elisa-client-api": {"repo_name": "elisa_client_api"},
    "connectivityserver": {"egg_name": "connection-service"},
}

packages_without_develop = {
    "moo": "master",
}
