#--------------------------------------------------------
# The CMakeLists.txt for:                     uSimCurrent
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
   nc_tools.h nc_tools.cpp
   USC_MOOSApp.h USC_MOOSApp.cpp
   USC_Info.h USC_Info.cpp
   main.cpp
)

ADD_EXECUTABLE(uSimROMS ${SRC})

message("${IVP_LIBRARY_DIRS}")
TARGET_LINK_LIBRARIES(uSimROMS
   ${MOOS_LIBRARIES}
   ${MOOSGeodesy_LIBRARIES}
   geometry
   mbutil
   ${SYSTEM_LIBS} 
  # /usr/local/lib/libnetcdf_c++.a
   netcdf  )

