dune_products_dirs=(
    "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/releases/dunedaq-v2.2.0/externals"
    "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/releases/dunedaq-v2.2.0/packages"
    #"/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products" 
    #"/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev" 
)

dune_products=(
    "cmake v3_17_2"
    "cetlib v3_11_01 e19:prof"
    "gdb v9_2"
    "TRACE v3_16_02"
    "folly v2020_05_25a e19:prof"
    "ers v0_26_00d e19:prof"
    "nlohmann_json v3_9_0c e19:prof"
    "ninja v1_10_0"
    "pistache v2020_10_07 e19:prof"
    "highfive v2_2_2b e19:prof"
    "zmq v4_3_1b e19"
    # Note: "daq_cmake" with underscore is the UPS product name.
    # One can use either "daq-cmake" or "daq_cmake" in this file.
    "daq-cmake v1_2_3 e19:prof"
    "cmdlib v1_0_2b e19:prof"
    "restcmd v1_0_3 e19:prof"
    "appfwk v2_1_0 e19:prof"
    "listrev v2_0_1b e19:prof"
    "dataformats v1_0_0 e19:prof"
    "dfmessages v1_0_0 e19:prof"
    "dfmodules v1_1_1 e19:prof"
    "trigemu v1_0_0 e19:prof"
    "readout v1_0_1 e19:prof"
    "minidaqapp v1_2_0 e19:prof"
    "ipm v1_1_0 e19:prof"
)

dune_python_version="v3_8_3b"
