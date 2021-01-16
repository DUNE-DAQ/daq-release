dune_products_dirs=(
    # "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/releases/dunedaq-v2.2.0/externals"
    # "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/releases/dunedaq-v2.2.0/packages"
    "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products" 
    "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev" 
)

dune_products=(
    "cmake v3_17_2"
    "gdb v9_2"
    "gcc v8_2_0 e19:prof"
    "boost v1_70_0 e19:prof"
    "cetlib v3_10_00 e19:prof"
    "TRACE v3_16_02"
    "folly v2020_05_25 e19:prof"
    "ers v0_26_00c e19:prof"
    "nlohmann_json v3_9_0b e19:prof"
    "ninja v1_10_0"
    "pistache v2020_10_07 e19:prof"
    "cmdlib v1_0_2 e19:prof"
    "restcmd v1_0_2 e19:prof"
    # Note: "daq_cmake" with underscore is the UPS product name.
    # One can use either "daq-cmake" or "daq_cmake" in this file.
    "daq-cmake v1_2_1 e19:prof"
    "appfwk v2_0_1 e19:prof"
    "listrev v2_0_1 e19:prof"
)

dune_python_version="v3_8_3b"
