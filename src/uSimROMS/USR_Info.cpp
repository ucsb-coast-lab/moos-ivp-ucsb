/*===================================================================
File: USR_Info.cpp
Authors: Nick Nidzieko & Sean Gillen
Date: Jan-23-15
Origin: Horn Point Laboratory
Description: uSimROMS Info file


Copyright 2015 Nick Nidzieko, Sean Gillen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===================================================================*/
#include <cstdlib>
#include <iostream>
#include "USR_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis()
{
  blk("SYNOPSIS:                                                       ");
  blk("------------------------------------                            ");
  blk("  The new uSimROMS3 application is a simple simulator of water  ");
  blk("  conditions. Based on a netCDF file that contains output from  ");
  blk("  ROMS it repeately reads the vehicle's present position and    ");
  blk("  takes a inverse weighted average of all nearby points         ");
  blk("                                                                ");
  blk("  Also implemented is a look forward function, which takes bathy");
  blk("  data from the NC file and checks a specified distance forward ");
  blk("  of the AUV for the minimum safe depth that can be expected.   ");
  blk("  the program simply publishes the variable, the user must check");
  blk("  it and take appropriate action themselves.                    ");
}

//----------------------------------------------------------------
// Procedure: showHelpAndExit

void showHelpAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("Usage: new uSimROMS3 file.moos [OPTIONS]                          ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("Options:                                                        ");
  mag("  --alias","=<ProcessName>                                      ");
  blk("      Launch new uSimROMS3 with the given process name rather     ");
  blk("      than new uSimROMS3.                                         ");
  mag("  --example, -e                                                 ");
  blk("      Display example MOOS configuration block.                 ");
  mag("  --help, -h                                                    ");
  blk("      Display this help message.                                ");
  mag("  --interface, -i                                               ");
  blk("      Display MOOS publications and subscriptions.              ");
  mag("  --version,-v                                                  ");
  blk("      Display the release version of uSimROMS.               ");
  blk("                                                                ");
  blk("Note: If argv[2] does not otherwise match a known option,       ");
  blk("      then it will be interpreted as a run alias. This is       ");
  blk("      to support pAntler launching conventions.                 ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showExampleConfigAndExit

void showExampleConfigAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("uSimCurrent Example MOOS Configuration                          ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = uSimCurrent                                     ");
  blk("{                                                               ");
  blk("  AppTick   = 4                                                 ");
  blk("  CommsTick = 4                                                 ");
  blk("                                                                ");
  blk("  NC_FILE_NAME = file.nc                                        ");
  blk("  OUTPUT_VARIABLE = SALINITY                                    ");
  blk("  SAFE_DEPTH_OUTPUT = SAFE_DEPTH                                ");
  blk("  SCALAR_VARIABLE = salt                                        ");
  blk("  BAD_VALUE = -1                                                ");
  blk("  LOOK_FORWARD = 20                                             ");
  blk("                                                                ");
  blk("                                                                ");
  blk("                                                                "); 
  blk("                                                                ");
  blk("}                                                               ");
  blk("                                                                ");
  exit(0);
}


//----------------------------------------------------------------
// Procedure: showInterfaceAndExit


void showInterfaceAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("uSimCurrent INTERFACE                                           ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("  NAV_X   = 123.4                                               ");
  blk("  NAV_Y   = -987.6                                              ");
  blk("  NAV_DEPTH = 0                                                 ");
  blk("  NAV_ALTITUDE = 50                                             ");
  blk("                                                                ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("  SCALAR_VALUE = 17.778                                         ");
  blk("                                                                ");
  blk("                                                                ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit()
{
  showReleaseInfo("new uSimROMS3  ", "gpl");
  exit(0);
}

