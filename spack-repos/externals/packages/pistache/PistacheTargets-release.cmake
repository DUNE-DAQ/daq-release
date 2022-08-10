#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "pistache_shared" for configuration "Release"
set_property(TARGET pistache APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(pistache PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libpistache.so.0.0.3"
  IMPORTED_SONAME_RELEASE "libpistache.so.0.0.3"
  )

list(APPEND _IMPORT_CHECK_TARGETS pistache )
list(APPEND _IMPORT_CHECK_FILES_FOR_pistache "${_IMPORT_PREFIX}/lib64/libpistache.so.0.0.3" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
