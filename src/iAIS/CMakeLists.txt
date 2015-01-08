PROJECT(iAIS)

#what files are needed?
SET(SRCS
 vdm_parse.h vdm_parse.cpp
 nmea.h nmea.cpp
 sixbit.h sixbit.cpp
 portable.h  
 tcpsocket.h tcpsocket.cpp
 CiAIS.h CiAIS.cpp
 CAIS.h CAIS.cpp
 iAISMain.cpp
)

SET(SYSTEM_LIBS
      m
      pthread )

ADD_EXECUTABLE(iAIS ${SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(iAIS 
   ${MOOSGeodesy_LIBRARIES}
   ${MOOS_LIBRARIES}
   ${SYSTEM_LIBS} )


INSTALL(TARGETS
iAIS
RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

