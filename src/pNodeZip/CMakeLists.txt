#--------------------------------------------------------
# The CMakeLists.txt for:                    uSimCurrent_Scalar
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
   NZ_MOOSApp.cpp
   NZ_MOOSApp.h
   main.cpp
   NZ_Info.cpp
   NZ_Info.h
)

ADD_EXECUTABLE(pNodeZip ${SRC})

message("${IVP_LIBRARY_DIRS}")
TARGET_LINK_LIBRARIES(pNodeZip
   ${MOOS_LIBRARIES}
   ${MOOSGeodesy_LIBRARIES}
   geometry
   mbutil
   ${SYSTEM_LIBS} 
   /usr/local/lib/libnetcdf_c++.a
   netcdf  )


# Install Targets
INSTALL(TARGETS pNodeZip
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

