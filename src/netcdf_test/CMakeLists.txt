#--------------------------------------------------------
# The CMakeLists.txt for:                    uSimCurrent2
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
   simple_xy_wr.cpp
)

ADD_EXECUTABLE(simple_xy_wr ${SRC})

TARGET_LINK_LIBRARIES(simple_xy_wr
   ${MOOSGeodesy_LIBRARIES}
   ${MOOS_LIBRARIES}
   ${SYSTEM_LIBS}    
   mbutil
   geometry  
   /usr/local/lib/libnetcdf_c++.a
   netcdf)


# Install Targets
INSTALL(TARGETS simple_xy_wr
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

