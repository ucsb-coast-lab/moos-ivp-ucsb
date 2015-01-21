#--------------------------------------------------------
# The CMakeLists.txt for:                       HPLUtils
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  NCData.cpp
  ReadNCFile.cpp
)

SET(HEADERS
  NCData.h
  )

# Build Library
ADD_LIBRARY(HPLUtils STATIC ${SRC})

TARGET_LINK_LIBRARIES(HPLUtils
  ${MOOS_LIBRARIES}
  ${MOOSGeodesy_LIBRARIES}
  geometry
  mbutil
  ${SYSTEM_LIBS}
  /usr/lib/libnetcdf_c++.a
  netcdf
  )
