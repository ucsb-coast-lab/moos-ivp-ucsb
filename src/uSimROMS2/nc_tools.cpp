/* This is part of the netCDF package.
        Copyright 2006 University Corporation for Atmospheric Research/Unidata.
        See COPYRIGHT file for conditions of use.
     
        This is a very simple example which writes a 2D array of
        sample data. To handle this in netCDF we create two shared
        dimensions, "x" and "y", and a netCDF variable, called "data".
     
        This example is part of the netCDF tutorial:
        http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial
     
        Full documentation of the netCDF C++ API can be found at:
        http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-cxx
     
        $Id: simple_xy_wr.cpp,v 1.15 2007/01/19 12:52:13 ed Exp $
*/
     
#include <cstdlib>
#include <string.h>     
#include "nc_tools.h"

using namespace std;
     
     
// Return this in event of a problem.
static const int NC_ERR = 2;


romsGrid read_nc_grid(std::string fileName,   std::string var)
{
  // This is the struct we will return
  romsGrid data;
 
  // Open the file. The ReadOnly parameter tells netCDF we want
  // read-only access to the file.
  NcFile dataFile(fileName.c_str(), NcFile::ReadOnly);
    
 
  // You should always check whether a netCDF file open or creation
  // constructor succeeded.
  if (!dataFile.is_valid())
    {
      cout << "***READ_NC: Error. Couldn't open file! " << fileName << endl;;
      return data;
    }
     

  // For other method calls, the default behavior of the C++ API is
  // to exit with a message if there is an error.  If that behavior
  // is OK, there is no need to check return values in simple cases
  // like the following.
     
  // Retrieve lat/long
  cout << "   Getting grid info for: " << var << endl;
  // Get the pointers to the variables
  char lonStr[10] = "lon_";
  char latStr[10] = "lat_";
  char xiStr[10] = "xi_";
  char etaStr[10] = "eta_";

  NcVar *lon_ = dataFile.get_var(strcat(lonStr,var.c_str()));
  NcVar *lat_ = dataFile.get_var(strcat(latStr,var.c_str()));

  // Figure out the dimensions
  NcDim *xi_ = dataFile.get_dim(strcat(xiStr,var.c_str()));
  NcDim *eta_ = dataFile.get_dim(strcat(etaStr,var.c_str()));

  int eta = eta_->size();
  int xi = xi_->size();

  data.xi = xi;
  data.eta = eta;

  cout << " readNC data structs" << endl;
  cout << "     -> eta_" << var << " : " << data.eta << endl;
  cout << "     ->  xi_" << var << " : " << data.xi << endl;

  // Initialize an array to hold coordinates
  double lat[eta][xi];    
  double lon[eta][xi];    
     
  // Get the data
  lon_->get(&data.lon[0][0], eta, xi);
  lat_->get(&data.lat[0][0], eta, xi);

  // The netCDF file is automatically closed by the NcFile destructor
  return data;
}
     

struct tm read_nc_time(std::string fileName,int pos)
{
  // This is the struct we will return
  struct tm data;
 
  // Open the file. The ReadOnly parameter tells netCDF we want
  // read-only access to the file.
  NcFile dataFile(fileName.c_str(), NcFile::ReadOnly);
    
  // You should always check whether a netCDF file open or creation
  // constructor succeeded.
  if (!dataFile.is_valid())
    {
      cout << "***READ_NC: Error. Couldn't open file! " << fileName << endl;;
      return data;
    }
       
 
  // Get the pointer to the variable
  NcVar *oceanTime_ = dataFile.get_var("ocean_time");

  // Figure out the dimensions
  NcDim *timeDim_ = dataFile.get_dim("ocean_time");

  if (pos == -1) {
    oceanTime_->set_cur(timeDim_->size() - 1);
    cout << "   Getting ocean_time[end]" << endl;
  }	
  else {
    oceanTime_->set_cur(pos);
    cout << "   Getting ocean_time[" << pos << "]" << endl;
  }
    

  // Get the time base
  NcAtt *timeBase_ = oceanTime_->get_att( "units" );
  std::string timeBase = timeBase_->as_string(0); 

  int year;
  int mon;
  int mday;
  int hour;
  int min;
  // Check that a valid time base string is returned
  if (timeBase.substr(0,13) == "seconds since") {
    year = atoi(timeBase.substr(14,4).c_str());
    mon = atoi(timeBase.substr(19,2).c_str());
    mday = atoi(timeBase.substr(22,2).c_str());
    hour = atoi(timeBase.substr(25,2).c_str());
    min = atoi(timeBase.substr(18,2).c_str());
  }


  cout << "Time base is: " << timeBase << endl;

  // Get the data
  oceanTime_->get(&data.tm_sec, 1);

  data.tm_year = year - 1900;
  data.tm_mon = mon - 1;
  data.tm_mday = mday - 1;
  data.tm_hour = hour;
  data.tm_min = min;

  // The netCDF file is automatically closed by the NcFile destructor
  return data;
}
     
