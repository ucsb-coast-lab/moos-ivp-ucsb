#--------------------------------------------------------
# The CMakeLists.txt for:                       pIncludeSampleData
# Author(s):                              cmoran
#--------------------------------------------------------

SET(SRC
  IncludeSampleData.cpp
  IncludeSampleData_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pIncludeSampleData ${SRC})

TARGET_LINK_LIBRARIES(pIncludeSampleData
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)

