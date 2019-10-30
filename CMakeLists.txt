cmake_minimum_required(VERSION 3.14)
project(centreonfs)

include(ExternalProject)

if (${BUILD_OFFLINE})
  set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY EP_UPDATE_DISCONNECTED 1)
endif ()

ExternalProject_Add(rest_lib
  GIT_REPOSITORY https://github.com/SylvestreG/libcrestapi
  TIMEOUT 10
  INSTALL_COMMAND ""
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON)

ExternalProject_Add(fuse_lib
  GIT_REPOSITORY https://github.com/jachappell/Fusepp
  TIMEOUT 10
  INSTALL_COMMAND ""
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON)

ExternalProject_Get_Property(rest_lib source_dir)
ExternalProject_Get_Property(rest_lib binary_dir)
link_directories(${binary_dir})
include_directories(${source_dir})

ExternalProject_Get_Property(fuse_lib source_dir)
ExternalProject_Get_Property(fuse_lib binary_dir)
link_directories(${binary_dir})
include_directories(${binary_dir}/include)

set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(fuse REQUIRED)
include_directories(${FUSE3_INCLUDE_DIRS})

add_definitions("-D_FILE_OFFSET_BITS=64")
add_executable(centreonfs main.cpp Fuse.cpp centreonfs.cpp)
add_dependencies(centreonfs rest_lib)

target_link_libraries(centreonfs -lcrestapi ${FUSE3_LIBRARIES})
