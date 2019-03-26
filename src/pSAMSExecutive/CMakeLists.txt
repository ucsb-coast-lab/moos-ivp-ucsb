#--------------------------------------------------------
# The CMakeLists.txt for:                       pSAMSExecutive
# Author(s):                              cmoran
#--------------------------------------------------------

cmake_minimum_required (VERSION 2.6)
project (pSAMSExecutive)

include_directories ("${PROJECT_BINARY_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}/parse_toml_rs/binding")

SET(SRC
  SAMSExecutive.cpp
  SAMSExecutive_Info.cpp
  main.cpp
  lib_mariner_sams.cpp
)

ADD_EXECUTABLE(pSAMSExecutive ${SRC})

include (ExternalProject)
set_directory_properties (PROPERTIES EP_PREFIX ${PROJECT_BINARY_DIR}/RUST)

ExternalProject_Add (
  parse_toml_rs
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND "" cargo build COMMAND cargo build --release COMMAND cbindgen -o binding/parse_toml_rs.h
  BINARY_DIR "${PROJECT_SOURCE_DIR}/parse_toml_rs"
  INSTALL_COMMAND ""
  LOG_BUILD ON )

add_dependencies(pSAMSExecutive parse_toml_rs)

TARGET_LINK_LIBRARIES(pSAMSExecutive
   ${MOOS_LIBRARIES}
   debug "${PROJECT_SOURCE_DIR}/parse_toml_rs/target/debug/libparse_toml_rs.a"
   optimized "${PROJECT_SOURCE_DIR}/parse_toml_rs/target/release/libparse_toml_rs.a"
   mbutil
   m
   dl
   pthread)

set_target_properties(pSAMSExecutive PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
