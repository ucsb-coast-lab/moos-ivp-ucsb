#ifndef USR_MOOSAPP_HEADER
#define USR_MOOSAPP_HEADER
#include "NCData.h"
#include <string>
#include <string.h>//not totally sure why this needs to be here, but strcmp isn't found when it's not
#include "MOOS/libMOOS/MOOSLib.h" 
#include "netcdfcpp.h"
#include "MOOSGeodesy.h"
#include "netcdf.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include <iostream>
#include <cmath>
#include <fstream>


class USR_MOOSApp : public CMOOSApp  
{
public:
  USR_MOOSApp();
  virtual ~USR_MOOSApp() {};

  bool Iterate();
  bool OnConnectToServer();
  bool OnDisconnectFromServer();
  bool OnStartUp();
  bool OnNewMail(MOOSMSG_LIST &NewMail);

 protected:
  void registerVariables();
 
 protected: // Configuration variables
  std::string ncFileName;
  std::string varName; 

  std::string scalarOutputVar;

 protected: // State variables
  // x/y positions, dpeth, and altitude of current location 
  double       m_posx;
  double       m_posy;
  double       m_depth;
  double       floor_depth;
  double       m_altitude;    

  //current depth level, and distances to nearest s_levels
  int          s_level;
  double       distSigma;
  double       distSp1;
  bool         above_s_level;
  
  //closest 4 eta/xi pairs(so closest_eta[0] and closest_xi[0] form one pair)and the respecitve distances to them
  int          closest_eta[4];
  int          closest_xi[4];
  double       closest_distance[4];
  double       closest_dist_meters[4];
  
  //our current lon/lat coordinate
  double       current_lat;
  double       current_lon;

  //the current time step, as well as the time from it and the time until the next one
  int          time_step;
  double       time_until;
  double       time_since;
  bool         more_time;
  bool         time_message_posted;

  //the value represnting a nonexistent value
  double       bad_val;
  //geodesy class
  CMOOSGeodesy geodesy;
  
  //stores variables from the CDF file
  double****   vals;
  int**        maskRho;
  double**     lat;
  double**     lon;
  double**     meters_n;
  double**     meters_e;
  double**     bathy;
  double*      time;
  double*      s_values;
  
  NCData ncdata;

  friend bool CMOOSGeodesy::LocalGrid2LatLong(double dfEast, double dfNorth, double &dfLat, double &dfLon) ;

};

 


#endif




