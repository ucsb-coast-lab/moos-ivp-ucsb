#--------------------------------------------------------
# The CMakeLists.txt for:                       iSerialMR
# Author(s):                                     nidzieko
#--------------------------------------------------------

FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(iSerialMR ${SRC})
   
TARGET_LINK_LIBRARIES(iSerialMR
   ${MOOS_LIBRARIES}
   anrp_util
   m
   pthread)
