#--------------------------------------------------------
# The CMakeLists.txt for:                       pLineTurn
# Author(s):                              cmoran
#--------------------------------------------------------

SET(SRC
  LineTurn.cpp
  LineTurn_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pLineTurn ${SRC})

TARGET_LINK_LIBRARIES(pLineTurn
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)

