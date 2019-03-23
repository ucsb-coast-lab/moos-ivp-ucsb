#--------------------------------------------------------
# The CMakeLists.txt for:                       pIncludeSampleData
# Author(s):                              cmoran
#--------------------------------------------------------

cmake_minimum_required (VERSION 2.6)
project (pIncludeSampleData)

include_directories ("${PROJECT_BINARY_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}/image_to_csv_rs/binding")

SET(SRC
  IncludeSampleData.cpp
  IncludeSampleData_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pIncludeSampleData ${SRC})

include (ExternalProject)
set_directory_properties (PROPERTIES EP_PREFIX ${PROJECT_BINARY_DIR}/RUST)

MESSAGE(STATUS "Should be about to execute Rust compilation and header creation in pIncludeSampleData")
ExternalProject_Add (
  image_to_csv_rs
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND cargo build COMMAND cargo build --release COMMAND cbindgen -o binding/image_to_csv_rs.h
  BINARY_DIR "${PROJECT_SOURCE_DIR}/image_to_csv_rs"
  INSTALL_COMMAND ""
  LOG_BUILD ON )

add_dependencies(pIncludeSampleData image_to_csv_rs)


# can use 'cargo rustc -- --print native-static-libs' to check for requisite dependencies
TARGET_LINK_LIBRARIES(pIncludeSampleData
   ${MOOS_LIBRARIES}
   debug "${PROJECT_SOURCE_DIR}/image_to_csv_rs/target/debug/libimage_to_csv_rs.a"
   optimized "${PROJECT_SOURCE_DIR}/image_to_csv_rs/target/release/libimage_to_csv_rs.a"
   dl
   mbutil
   m
   pthread
)
