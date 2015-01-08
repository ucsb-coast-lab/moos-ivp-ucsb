#--------------------------------------------------------
# The CMakeLists.txt for:                    uSendModemMsg
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
   PSM_MOOSApp.h PSM_MOOSApp.cpp
   main.cpp
)

ADD_EXECUTABLE(pSendModemMsg ${SRC})

message("${IVP_LIBRARY_DIRS}")
TARGET_LINK_LIBRARIES(pSendModemMsg
   ${MOOS_LIBRARIES}
   ${MOOSGeodesy_LIBRARIES}
   geometry
   mbutil
   ${SYSTEM_LIBS} 
    )


# Install Targets
INSTALL(TARGETS pSendModemMsg
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

