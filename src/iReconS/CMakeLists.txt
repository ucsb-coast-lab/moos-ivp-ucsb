PROJECT(iRecon)

#what files are needed?
SET(SRCS
 ReconS.h ReconS.cpp
 ReconConnectionS.h ReconConnectionS.cpp
 ntpshmS.h ntpshmS.cpp
 iReconS.cpp
)

ADD_EXECUTABLE(iReconS ${SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(iReconS 
   ${MOOSGeodesy_LIBRARIES}
   ${MOOS_LIBRARIES}
   ${SYSTEM_LIBS} )
#TARGET_LINK_LIBRARIES(iReconS MOOS MOOSGen MOOSUtility pthread)

INSTALL(TARGETS
iReconS
RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

