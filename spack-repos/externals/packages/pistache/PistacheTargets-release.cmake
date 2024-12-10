#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "pistache_shared" for configuration "Release"
set_property(TARGET pistache_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(pistache_shared PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libpistache.so.0.4.23"
  IMPORTED_SONAME_RELEASE "libpistache.so.0.4.23"
  )

list(APPEND _IMPORT_CHECK_TARGETS pistache_shared )
list(APPEND _IMPORT_CHECK_FILES_FOR_pistache_shared "${_IMPORT_PREFIX}/lib64/libpistache.so.0.4.23" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
