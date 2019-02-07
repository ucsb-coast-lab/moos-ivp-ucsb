#--------------------------------------------------------
# The CMakeLists.txt for:                       pSimDistanceGenerator
# Author(s):                              cmoran
#--------------------------------------------------------

SET(SRC
  SimDistanceGenerator.cpp
  SimDistanceGenerator_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pSimDistanceGenerator ${SRC})

TARGET_LINK_LIBRARIES(pSimDistanceGenerator
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)

