#--------------------------------------------------------
# The CMakeLists.txt for:                    uSendModemMsg
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
   USM_MOOSApp.h USM_MOOSApp.cpp
   main.cpp USM_Info.cpp USM_Info.h
)

ADD_EXECUTABLE(uSendModemMsg ${SRC})

message("${IVP_LIBRARY_DIRS}")
TARGET_LINK_LIBRARIES(uSendModemMsg
   ${MOOS_LIBRARIES}
   ${MOOSGeodesy_LIBRARIES}
   geometry
   mbutil
   ${SYSTEM_LIBS} 
   /usr/local/lib/libnetcdf_c++.a
   netcdf  )


# Install Targets
INSTALL(TARGETS uSendModemMsg
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

