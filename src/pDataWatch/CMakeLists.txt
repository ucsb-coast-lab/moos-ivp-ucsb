#--------------------------------------------------------
# The CMakeLists.txt for:                   pDataWatch
# Author(s):                                Mike Benjamin
#--------------------------------------------------------

# Set System Specific Libraries
if (${WIN32})
  SET(SYSTEM_LIBS
    wsock32)
else (${WIN32})
  SET(SYSTEM_LIBS
    m
    pthread)
endif (${WIN32})

SET(SRC
   pDataWatch.h pDataWatch.cpp
   main.cpp
)

ADD_EXECUTABLE(pDataWatch ${SRC})

message("${IVP_LIBRARY_DIRS}")
TARGET_LINK_LIBRARIES(pDataWatch
   ${MOOS_LIBRARIES}
   ${MOOSGeodesy_LIBRARIES}
   geometry
   mbutil
   ${SYSTEM_LIBS} 
   /usr/local/lib/libnetcdf_c++.a
   netcdf  )


# Install Targets
INSTALL(TARGETS pDataWatch
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

