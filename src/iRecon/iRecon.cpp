/**
 * iRECON.cpp
 *
 * Startup code for REMUS RECON Interface
 *
 */
#include <iostream>
#include <limits.h>
#include <signal.h>
#include <syslog.h>
#include "Recon.h"
using namespace std;

/**
 * Main used for starting up RECON class object
 */
int main(int argc, char **argv)
{
  Recon app;
  
  // Set up default app parameters
  const char *sMissionFile = "Mission.moos"; // Default MOOSDB application config file
  const char *sMOOSName    = "iRECON";       // Default MOOSDB application name
  
  // Override defaults from command line
  switch(argc)
  {
  case 3:
    //command line says don't register with default name
    sMOOSName = argv[2];
  case 2:
    //command line says don't use default "mission.moos" config file
    sMissionFile = argv[1];
  }
  
  // Instantiate the application, and run it
  app.Run(sMOOSName,sMissionFile);
  
  // Will get here only if app exits
  return 0;
}
