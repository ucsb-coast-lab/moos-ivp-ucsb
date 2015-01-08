#!/bin/bash

PWD=`pwd`
BUILD_DIR=./build
HELP="no"
CLEAN="no"

#-------------------------------------------------------------------
# Define some terminal colors for convenience
#-------------------------------------------------------------------

txtrst=$(tput setaf 0)  # Reset
txtred=$(tput setaf 1)  # Red
txtgrn=$(tput setaf 2)  # Green
txtylw=$(tput setaf 3)  # Yellow
txtblu=$(tput setaf 4)  # Blue
txtpur=$(tput setaf 5)  # Purple
txtcyn=$(tput setaf 6)  # Cyan
txtwht=$(tput setaf 7)  # White


#-------------------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" ] ; then
	HELP="yes"
    fi
    if [ "${ARGI}" = "-h" ] ; then
	HELP="yes"
    fi
    if [ "${ARGI}" = "--clean" ] ; then
	CLEAN="yes"
    fi
    if [ "${ARGI}" = "clean" ] ; then
	CLEAN="yes"
    fi
done

#-------------------------------------------------------------------
#  Part 2: Handle the request for help
#-------------------------------------------------------------------

if [ "${HELP}" = "yes" ]; then
    printf "${txtblu}%s [SWITCHES] ${txtrst}    \n" $0
    printf "Switches:                           \n" 
    printf "  --clean, clean                    \n" 
    printf "  --help, -h                        \n" 
    printf "Notes:                              \n"
    printf " (1) All other command line args will be passed as args    \n"
    printf "     to \"make\" when it is eventually invoked.            \n"
    printf " (2) For example -k will continue making when/if a failure \n"
    printf "     is encountered in building one of the subdirectories. \n"
    printf " (3) For example -j2 will utilize a 2nd core in the build  \n"
    printf "     if your machine has two cores. -j4 etc for quad core. \n"
    exit 0;
fi


#-------------------------------------------------------------------
#  Part 3: Handle a request to for make clean. Most cleaning handled
#          by the invocation of make clean, but some extras are done.
#-------------------------------------------------------------------

if [ "${CLEAN}" = "yes" ]; then
    cd ${BUILD_DIR}
    printf "Cleaning the project....."
    make clean
    printf "Done.\n"
    cd ${PWD}
    exit 0;
fi

#-------------------------------------------------------------------
#  Part 4: Invoke the call to make in the build directory
#-------------------------------------------------------------------

cd ${BUILD_DIR}
cmake ..
make $@
cd ${PWD}


############################################################
#                   END of build.sh
############################################################

