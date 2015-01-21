#ifndef NCDATA_HEADER
#define NCDATA_HEADER

#include "MOOS/libMOOS/MOOSLib.h" 
#include "netcdfcpp.h"
#include "MOOSGeodesy.h"
#include "Utils.h"

class NCData  
{
public:
  NCData();
  virtual ~NCData() {};
  bool Initialise(double, double, std::string, std::string, std::string);
  bool LatLontoIndex(double, double);
  bool LatlontoMeters();
  bool Update(double x, double y , double h , double time);
  //getters
  double GetValue();
  double GetFloorDepth();
  double GetAltitude();
private: 
  bool GetS_rho(double depth, double altitude);
  double CalcValue();
  bool ReadNcFile(std::string ncFileName, std::string varName); //this is defined in a seperate file 
  double GetValueAtTime(int);
  bool GetTimeInfo(double time);
  bool GetBathy();
  bool GetSafeDepth();
  bool ConvertToMeters();
  

 protected: // Configuration variables 
  std::string maskRhoVarName;
  std::string latVarName;
  std::string lonVarName;
  std::string timeVarName;
  std::string sVarName;
  std::string bathyVarName;
  std::string scalarOutputVar;
  std::string safeDepthVar;

  std::string debugName;

  int time_vals; //number of time vals
  int s_rho;  //number of s_rho points
  int eta_rho;//number of eta_rho points
  int xi_rho; //number of xi_rho points

 protected: // State variables

  double start_time;
  double current_time;

  double       m_depth;
  double       floor_depth;
  double       m_altitude;
  double       m_value;

  //current depth level, and distances to nearest s_levels
  int          s_level;
  double       distSigma;
  double       distSp1;
  bool         above_s_level;
  
  //closest 4 eta/xi pairs(so closest_eta[0] and closest_xi[0] form one pair)and the respecitve distances to them
  int          eta[4];
  int          xi[4];
  double       distance[4];
  double       dist[4];
  
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
  


  friend bool CMOOSGeodesy::LocalGrid2LatLong(double dfEast, double dfNorth, double &dfLat, double &dfLon) ;

};

#endif




