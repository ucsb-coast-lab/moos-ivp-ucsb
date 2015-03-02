/*===================================================================
File: NCData.h
Authors: Nick Nidzieko & Sean Gillen
Date: Jan-23-2015
Origin: Horn Point Laboratory
Description: a class to deal with ROMS data, see the .cpp file for
             more details. 

Copyright 2015 Nick Nidzieko, Sean Gillen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===================================================================*/

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
  bool Initialise(double, double, std::string, std::string, std::string*, std::string);
  bool XYtoIndex(int l_eta[4], int l_xi[4], double l_dist[4], double x , double y, double **l_meters_e, double **l_meters_n,
		 int size_eta, int size_xi );
  bool LatLontoMeters();
  bool Update(double x, double y , double h , double time);
  //getters
  double GetValue();
  double GetFloorDepth();
  double GetAltitude();
protected: 
  bool GetS_rho(double depth, double altitude);
  double CalcValue();
  bool ReadNcFile(std::string ncFileName, std::string varName, std::string *vecVarName); //this is defined in a seperate file 
  double GetValueAtTime(int);
  bool GetTimeInfo(double time);
  bool GetBathy();
  bool GetSafeDepth();
  bool ConvertToMeters(double*** northings, double*** eastings, double** lat_l , double** lon_l , int eta, int xi);

  bool readScalarVar(std::string varName, NcFile *pFile);
  bool readVectorVar(std::string vecVarName[3], NcFile *pFile);
		     
  NcVar* findNcVar(std::string, NcFile*); 
  double**** readNcVar4(NcVar* , long size[4]);
  double**   readNcVar2(NcVar* , long size[4]);
 

 protected: // Configuration variables 
  std::string maskRhoVarName;
  
  std::string latVarName;
  std::string lonVarName;
  std::string lat_vVarName;
  std::string lon_vVarName;
  std::string lat_uVarName;
  std::string lon_uVarName;
  
  std::string timeVarName;
  std::string sVarName;
  std::string bathyVarName;
  std::string scalarOutputVar;
  std::string debugName;

  int time_vals; //number of time vals
  int s_rho;  //number of s_rho points
  int eta_rho;//number of eta_rho points
  int xi_rho; //number of xi_rho points

  //number of u/v points
  int eta_u;
  int eta_v;
  int xi_v;
  int xi_u;

 protected: // State variables

  double start_time;
  double current_time;

  double       m_depth;
  double       floor_depth;
  double       m_altitude;
  
  double       m_value;
  double       m_vec_mag;
  double       m_angle;
  //current depth level, and distances to nearest s_levels
  int          s_level;
  double       distSigma;
  double       distSp1;
  bool         above_s_level;
  
  //closest 4 eta/xi pairs(so eta[0] and xi[0] form one pair)and the respecitve distances to them
  int          eta_rho_index[4];
  int          xi_rho_index[4];  
  double       dist[4];

  int          eta_u_index[4];
  int          xi_u_index[4];
  double       dist_u[4];

  int          eta_v_index[4];
  int          xi_v_index[4];
  double       dist_v[4];

  int          eta_w_index[4];
  int          xi_w_index[4];
  double       dist_w[4];  //should be the same as dist
  
  //our current lon/lat coordinate
  double       current_lat;
  double       current_lon;

  //the current time step, as well as the time from it and the time until the next one
  int          time_step;
  double       time_until;
  double       time_since;
  bool         more_time;
  bool         time_message_posted;

  //the value representing a nonexistent value
  double       bad_val;
  //geodesy class
  CMOOSGeodesy geodesy;
  
  //stores variables from the CDF file
  double****   vals;
  double**     maskRho;
  double**     lat;
  double**     lon;
  double**     meters_n;
  double**     meters_e;
  double**     bathy;
  double*      time;
  double*      s_values;

  double****   uVals;
  double****   vVals;
  double****   wVals;
  
  double**     uLat;
  double**     uLon;
  double**     u_meters_n;
  double**     u_meters_e;
  
  double**     vLat;
  double**     vLon;
  double**     v_meters_n;
  double**     v_meters_e;
  
  friend bool CMOOSGeodesy::LocalGrid2LatLong(double dfEast, double dfNorth, double &dfLat, double &dfLon) ;

};

#endif




