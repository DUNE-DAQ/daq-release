dune_products_dirs=(
    "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/releases/dunedaq-v2.3.0/externals"
    "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/releases/dunedaq-v2.3.0/packages"
    #"/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products" 
    #"/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev" 
)

dune_systems=(
    "gcc v8_2_0"
    "python v3_8_3b"
    )

dune_devtools=(
    "cmake v3_17_2"
    "gdb v9_2"
    "ninja v1_10_0"
    )

dune_externals=(
    "cetlib v3_11_01 e19:prof"
    "TRACE v3_16_02"
    "folly v2020_05_25a e19:prof"
    "nlohmann_json v3_9_0c e19:prof"
    "pistache v2020_10_07 e19:prof"
    "highfive v2_2_2b e19:prof"
    "zmq v4_3_1c e19:prof"
    "cppzmq v4_3_0 e19:prof"
    "msgpack_c v3_3_0 e19:prof"
    )

dune_daqpackages=(
    # Note: "daq_cmake" with underscore is the UPS product name.
    # One can use either "daq-cmake" or "daq_cmake" in this file.
    "daq-cmake v1_3_2 e19:prof"
    "appfwk v2_2_0 e19:prof"
    "cmdlib v1_1_1 e19:prof"
    "restcmd v1_1_1 e19:prof"
    "listrev v2_1_1 e19:prof"
    "ers v1_1_1 e19:prof"
    "logging v1_0_1 e19:prof"
    "opmonlib v1_0_0 e19:prof"
    "rcif v1_0_1 e19:prof"
)
