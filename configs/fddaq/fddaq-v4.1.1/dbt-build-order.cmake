# "build_order" lists packages in the order you'd want CMake to see
# them (via "add_subdirectory") during a simultaneous build. This is
# due to their dependencies: e.g., you'd want CMake to see
# daq-buildtools first in order to create daq-buildtoolsConfig.cmake
# so "find_package(daq-builtools)" will work for all the other
# packages, and so on. If a new package is introduced into the
# development area, the developer is encouraged to add it to its
# appropriate place in this list


set(build_order "daq-cmake"
                "ers"
                "erskafka"
                "logging"
                "opmonlib"
                "cmdlib"
                "rcif"
                "restcmd"
                "utilities"
                "ipm"
                "serialization"
                "iomanager"
		"okssystem"
		"oksdbinterfaces"
		"oks"
		"genconfig"
		"oksutils"
		"dal"
		"coredal"
		"oksconfig"
		"dbe"
                "appfwk"
		"hermesmodules"
                "daqconf"
                "listrev"
                "detdataformats"
		"trgdataformats"
                "daqdataformats"
                "detchannelmaps"
                "dfmessages"
                "triggeralgs"
                "timing"
                "timinglibs"
                "hdf5libs"
                "readoutlibs"
                "hsilibs"
                "trigger"
                "readoutmodules"
                "dfmodules"
                "kafkaopmon"
		# FD packages
                "fddetdataformats"
                "fdreadoutlibs"
                "fdreadoutmodules"
                "flxlibs"
                "dqm"
                "wibmod"
                "sspmodules"
                "uhallibs"
                "dtpcontrols"
                "rawdatautils"
                "dtpctrllibs"
                "ctbmodules"
                "dpdklibs"
		# ND packages
                "nddetdataformats"
                "ndreadoutlibs"
                "ndreadoutmodules"
                "lbrulibs"
)

