##############################################################################
# FILE:        moos-ivp-hpl/src/CMakeLists.txt
# DATE:        2010/09/07
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-hpl source directory
##############################################################################


#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_* )
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})


find_package(MOOSGeodesy)
include_directories(${MOOSGeodesy_INCLUDE_DIRS} )
link_directories(${MOOSGeodesy_LIBRARY_PATH})

#============================================================================
# List the subdirectories to build...
#============================================================================
 ADD_SUBDIRECTORY(iAIS)
 ADD_SUBDIRECTORY(iRecon)
 ADD_SUBDIRECTORY(iGPSd)
 ADD_SUBDIRECTORY(pDataWatch)
 ADD_SUBDIRECTORY(uSimROMS3)
 ADD_SUBDIRECTORY(iWhoiMicroModem)
 ADD_SUBDIRECTORY(lib_YellowSubUtils)
 ADD_SUBDIRECTORY(uSendModemMsg)
 ADD_SUBDIRECTORY(pNodeZip)
##############################################################################
#                           END of CMakeLists.txt
##############################################################################

