##############################################################################
# FILE:        moos-ivp-extend/src/CMakeLists.txt
# DATE:        2010/09/07
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-extend source directory
##############################################################################


#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_*)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})

#============================================================================
# List the subdirectories to build...
#============================================================================

#ADD_SUBDIRECTORY(iAIS)
#ADD_SUBDIRECTORY(iRecon)
#ADD_SUBDIRECTORY(iSerialMR)
#ADD_SUBDIRECTORY(iWhoiMicroModem)
#ADD_SUBDIRECTORY(lib_anrp_util)
#ADD_SUBDIRECTORY(lib_behavior-test)
#ADD_SUBDIRECTORY(lib_YellowSubUtils)
#ADD_SUBDIRECTORY(pDataWatch)

ADD_SUBDIRECTORY(pIncludeSampleData)
ADD_SUBDIRECTORY(pLineFollow)
ADD_SUBDIRECTORY(pLineTurn)
ADD_SUBDIRECTORY(pSAMSExecutive)
ADD_SUBDIRECTORY(pSimDistanceGenerator)

#ADD_SUBDIRECTORY(pNodeZip)
#ADD_SUBDIRECTORY(pSendModemMsg)
#ADD_SUBDIRECTORY(uSimROMS3)


##############################################################################
#                           END of CMakeLists.txt
##############################################################################

