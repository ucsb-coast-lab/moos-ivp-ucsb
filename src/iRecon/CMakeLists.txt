PROJECT(iRecon)

#what files are needed?
SET(SRCS
 Recon.h Recon.cpp
 ReconConnection.h ReconConnection.cpp
 ntpshm.h ntpshm.cpp
 iRecon.cpp
)

ADD_EXECUTABLE(iRecon ${SRCS})

# indicate how to link
# TARGET_LINK_LIBRARIES(iRecon MOOS MOOSGen MOOSUtility pthread)
TARGET_LINK_LIBRARIES(iRecon 
   ${MOOSGeodesy_LIBRARIES}
   ${MOOS_LIBRARIES}
   ${SYSTEM_LIBS} )

INSTALL(TARGETS
iRecon
RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

