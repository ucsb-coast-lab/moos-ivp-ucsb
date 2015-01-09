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
   USR_MOOSApp.h USR_MOOSApp.cpp
   USR_Info.h USR_Info.cpp
   main.cpp USR_ReadNCFile.cpp
)

ADD_EXECUTABLE(uSimROMS3 ${SRC})

message("${IVP_LIBRARY_DIRS}")
TARGET_LINK_LIBRARIES(uSimROMS3
   ${MOOS_LIBRARIES}
   ${MOOSGeodesy_LIBRARIES}
   geometry
   mbutil
   ${SYSTEM_LIBS}
   /usr/lib/libnetcdf_c++.a
   netcdf  )


# Install Targets
INSTALL(TARGETS uSimROMS3
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

