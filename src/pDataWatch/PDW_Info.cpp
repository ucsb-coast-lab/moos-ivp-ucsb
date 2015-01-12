#include <cstdlib>
#include <iostream>
#include "PDW_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis()
{
  blk("SYNOPSIS:                                                       ");
  blk("------------------------------------                            ");
  blk(" pDataWatch is a utility for analyzing data collected in real   ");
  blk(" time to adaptively change behavior mid-mission. at present     ");
  blk(" the app is hard coded to bounce back and forth between two     ");
  blk(" thresholds of salinity or some other scalar variable. In the   ");
  blk(" Future we may make the app more general.                       ");

}

//----------------------------------------------------------------
// Procedure: showHelpAndExit

void showHelpAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("Usage: new pDataWatch file.moos [OPTIONS]                          ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("Options:                                                        ");
  mag("  --alias","=<ProcessName>                                      ");
  blk("      Launch new pDatawatch with the given process name rather     ");
  blk("      than new pDatawatch.                                         ");
  mag("  --example, -e                                                 ");
  blk("      Display example MOOS configuration block.                 ");
  mag("  --help, -h                                                    ");
  blk("      Display this help message.                                ");
  mag("  --interface, -i                                               ");
  blk("      Display MOOS publications and subscriptions.              ");
  mag("  --version,-v                                                  ");
  blk("      Display the release version of pDatawatch.               ");
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
  blu("pDatawatch Example MOOS Configuration                          ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = pDataWatch                                      ");
  blk("{                                                               ");
  blk("  AppTick   = 4                                                 ");
  blk("  CommsTick = 4                                                 ");
  blk("                                                                ");
  blk("  PAPA_TARGET = 8.2                                             ");
  blk("  ECHO_TARGET = 8.7                                             ");
  blk("  SCALAR_SOURCE = SALINITY                                      ");
  blk("  POINTS_AVERAGED = 300                                         ");
  blk("  TIME_TO_EXCEED = 10                                           ");
  blk("                                                                ");
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
  blu("pDataWatch  INTERFACE                                           ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("  [value of SCALAR_SOURCE]                                      ");
  blk("  NAV_DEPTH                                                     ");
  blk("  ON_PAPA                                                       ");
  blk("                                                                ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("  ON_PAPA                                                       ");
  blk("                                                                ");
  blk("                                                                ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit()
{
  showReleaseInfo("new pDataWatch  ", "gpl");
  exit(0);
}

