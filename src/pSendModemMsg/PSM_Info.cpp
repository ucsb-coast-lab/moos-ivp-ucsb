#include <cstdlib>
#include <iostream>
#include "PSM_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis()
{
  blk("SYNOPSIS:                                                       ");
  blk("------------------------------------                            ");
  blk("pSendModemMsg is a utility for sending a variable from the  ");
  blk("MOOS DB over an acoustic modem. It must be run in parallel with ");
  blk("the iWhoiMicroModem process. Supports packet types 0,2,and 6.   ");
  blk("The process converts your string into hex for you.              ");
}

//----------------------------------------------------------------
// Procedure: showHelpAndExit

void showHelpAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("Usage: new pSendModemMsg file.moos [OPTIONS]                    ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("Options:                                                        ");
  mag("  --alias","=<ProcessName>                                      ");
  blk("      Launch new pSendModemMsg with the given process name rather");
  blk("      than new pSendModemMsg.                                    ");
  mag("  --example, -e                                                 ");
  blk("      Display example MOOS configuration block.                 ");
  mag("  --help, -h                                                    ");
  blk("      Display this help message.                                ");
  mag("  --interface, -i                                               ");
  blk("      Display MOOS publications and subscriptions.              ");
  mag("  --version,-v                                                  ");
  blk("      Display the release version of pSendModemMsg.               ");
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
  blu("pSendModemMsg Example MOOS Configuration                        ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = pSendModemMsg                                   ");
  blk("{                                                               ");
  blk("  AppTick   = 4                                                 ");
  blk("  CommsTick = 4                                                 ");
  blk("                                                                ");
  blk("  DEST = 8                                                      ");
  blk("  PACKETTYPE = 0                                                ");
  blk("  ACKTIMEOUTSEC = 5                                             ");
  blk("  Var1 = ZIPPED_REPORT                                          ");
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
  blu("pSendModemMsg INTERFACE                                     ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("  [Value of Var1]                                               ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("  iWhoiMicroModem_CMD                                           ");
  blk("                                                                ");
  blk("                                                                ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit()
{
  showReleaseInfo("new pSendModemMsg  ", "gpl");
  exit(0);
}

