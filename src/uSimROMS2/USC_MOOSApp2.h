
#ifndef USC_MOOSAPP_HEADER
#define USC_MOOSAPP_HEADER

#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "CurrentField.h"
#include "netcdfcpp.h"
#include "MOOSGeodesy.h"
#include "netcdf.h"


class USC_MOOSApp : public CMOOSApp  
{
public:
  USC_MOOSApp();
  virtual ~USC_MOOSApp() {};

  bool Iterate();
  bool OnConnectToServer();
  bool OnDisconnectFromServer();
  bool OnStartUp();
  bool OnNewMail(MOOSMSG_LIST &NewMail);

 protected:
  void registerVariables();
  bool ReadNcFile(); 
  bool LatLontoIndex();
  bool GetS_rho();
  double GetValue();
  double GetValueAtTime(int);
  double WeightedAvg(double*,double*, int);
  bool GetTimeInfo();
  bool GetBathy();



 protected: // Configuration variables
  std::string ncFileName;
  std::string varName; 
  std::string latVarName;
  std::string lonVarName;
  std::string timeVarName;
  std::string sVarName;
  std::string bathyVarName;
  std::string scalarOutputVar;

  int time_vals; //number of time vals
  int s_rho;  //number of s_rho points
  int eta_rho;//number of eta_rho points
  int xi_rho; //number of xi_rho points

  //determines if the user wants to override the size of the matrix
  bool time_override;
  bool s_override;
  bool eta_override;
  bool xi_override;
  
  bool bathy_only;

 protected: // State variables

  //stores whether we have a bottom lock or not 
  bool bottom_lock;

  double start_time;
  double current_time;

  // x/y positions, dpeth, and altitude of current location 
  double       m_posx;
  double       m_posy;
  double       m_depth;
  double       m_altitude;    

  //current depth level, and distances to nearest s_levels
  int          s_level;
  double       dist_to_s_level;
  double       dist_to_next_s_level;
  bool         above_s_level;
  
  //closest 4 eta/xi pairs, and the respecitve distances to them
  int          closest_eta[4];
  int          closest_xi[4];
  double       closest_distance[4];
  
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
  double**     lat;
  double**     lon;
  double**     bathy;
  double*      time;
  double*      s_values;


  friend bool CMOOSGeodesy::LocalGrid2LatLong(double dfEast, double dfNorth, double &dfLat, double &dfLon) ;
};

 


#endif




