#include "MOOSLib.h"
#include "CiAIS.h" // definition for the iAIS class

int main(int argc ,char * argv[])
{
  const char * sMissionFile = "iAIS.moos";

  if(argc>1)
    {
      sMissionFile = argv[1];
    }


  // Declare a CiAIS class variable
  CiAIS iAIS;

  // Run that instance
  iAIS.Run("iAIS", sMissionFile);


  return 0;
}


