#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "RdKafka::rdkafka" for configuration ""
set_property(TARGET RdKafka::rdkafka APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(RdKafka::rdkafka PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib64/librdkafka.so.1"
  IMPORTED_SONAME_NOCONFIG "librdkafka.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS RdKafka::rdkafka )
list(APPEND _IMPORT_CHECK_FILES_FOR_RdKafka::rdkafka "${_IMPORT_PREFIX}/lib64/librdkafka.so.1" )

# Import target "RdKafka::rdkafka++" for configuration ""
set_property(TARGET RdKafka::rdkafka++ APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(RdKafka::rdkafka++ PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib64/librdkafka++.so.1"
  IMPORTED_SONAME_NOCONFIG "librdkafka++.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS RdKafka::rdkafka++ )
list(APPEND _IMPORT_CHECK_FILES_FOR_RdKafka::rdkafka++ "${_IMPORT_PREFIX}/lib64/librdkafka++.so.1" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
