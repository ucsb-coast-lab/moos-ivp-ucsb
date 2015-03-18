// This is a mashup/mod of the iMOOS2Serial class 
// written by Andrew Patikalakis.
// nidzieko@umces.edu
// 18March2015
//

#include "CiSerialMR.h"

int main(int argc, char *argv[])
{
	const char *sMissionFile = "iSerialMR.moos";
	
	if(argc > 1) {
		sMissionFile = argv[1];
	}
	
	CiSerialMR iSerialMR;
	
	iSerialMR.Run("iSerialMR", sMissionFile);

	return 0;
}

