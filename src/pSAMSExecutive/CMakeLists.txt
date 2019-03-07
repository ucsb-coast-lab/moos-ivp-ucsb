#--------------------------------------------------------
# The CMakeLists.txt for:                       pSAMSExecutive
# Author(s):                              cmoran
#--------------------------------------------------------

SET(SRC
  SAMSExecutive.cpp
  SAMSExecutive_Info.cpp
  main.cpp
  lib_mariner_sams.cpp
)

ADD_EXECUTABLE(pSAMSExecutive ${SRC})

TARGET_LINK_LIBRARIES(pSAMSExecutive
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)
