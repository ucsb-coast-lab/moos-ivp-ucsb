

#ifndef WRITE_NC_HEADER
#define WRITE_NC_HEADER

#include <netcdfcpp.h>
#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"


struct romsGrid {
  int xi;
  int eta;
  double lon[200][200];
  double lat[200][200];
} ;

struct timeStruct {
  int time;
  int oceanTime[200];
} ;


romsGrid read_nc_grid(std::string fileName, std::string varRoot);
struct tm read_nc_starttime(std::string fileName);
struct tm read_nc_endtime(std::string fileName);



#endif
