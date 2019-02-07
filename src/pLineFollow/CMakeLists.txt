#--------------------------------------------------------
# The CMakeLists.txt for:                       pLineFollow
# Author(s):                              cmoran
#--------------------------------------------------------

SET(SRC
  LineFollow.cpp
  LineFollow_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pLineFollow ${SRC})

TARGET_LINK_LIBRARIES(pLineFollow
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)

