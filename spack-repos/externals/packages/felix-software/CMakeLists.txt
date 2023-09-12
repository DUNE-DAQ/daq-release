cmake_minimum_required(VERSION 3.4.3)

set(PACKAGE packetformat)
set(PACKAGE_VERSION 0.0.1)

include(FELIX)

felix_add_external(catch ${catch_version})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g -O3")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    message(FATAL_ERROR "GCC version must be at least 4.8!")
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qopt-prefetch -unroll-aggressive -march=native -mtune=native")
endif()

file(GLOB PACKETFORMAT_SOURCES src/*.cpp)
tdaq_add_library(packetformat ${PACKETFORMAT_SOURCES})

#file(GLOB PACKETFORMAT_TEST_SOURCES test/*.cpp)
#tdaq_add_executable(packetformat_test ${PACKETFORMAT_TEST_SOURCES})
#target_link_libraries(packetformat_test	packetformat)
#add_test(felix-packetformat-test packetformat_test)

tdaq_add_executable(packetformat_dump_subchunks src/main_dump_subchunks.cpp)
target_link_libraries(packetformat_dump_subchunks	packetformat)

#add_custom_target(felix-packetformat-tests ALL
#	COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test ${CMAKE_CURRENT_BINARY_DIR}/test)


install(TARGETS packetformat EXPORT felix DESTINATION lib)
