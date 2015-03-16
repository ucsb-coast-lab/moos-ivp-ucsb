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

//TODO : use a consistent naming convention 

#ifndef NCDATA_HEADER
#define NCDATA_HEADER

#include "MOOS/libMOOS/MOOSLib.h" 
#include "netcdfcpp.h"
#include "MOOSGeodesy.h"
#include "Utils.h"
#include "math.h"

class NCData  
{
public:
  //TODO: clean up variables that really don't need to be members
  //TODO: change method names to consistent naming convention
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
  bool getS_rho(double depth, double altitude);
  double calcValue(int eta_index[4], int xi_index[4], double dist[4], double ****vals);
  bool ReadNcFile(std::string ncFileName, std::string varName, std::string *vecVarName); //this is defined in a seperate file 
  double getValueAtTime(int time, int xi_index[4] , int eta_index[4], double dist[4], double**** vals);
  bool getTimeInfo(double time);
  bool GetBathy();
  bool GetSafeDepth();
  bool convertToMeters(double*** northings, double*** eastings, double** lat_l , double** lon_l , int eta, int xi);

  bool readScalarVar(std::string varName, NcFile *pFile);
  bool readVectorVar(std::string vecVarName[3], NcFile *pFile);
		     
  NcVar*     findNcVar(std::string, NcFile*); 
  double**** readNcVar4(NcVar* , long size[4]);
  double**   readNcVar2(NcVar* , long size[4]);
  bool       convertToEastNorth(double *****pvals_east , double *****pvals_north, long size[4], double ****vals, double **angle);
  double**** combineVectorVals(double ****u_vals_north, double ****v_vals_east, long size_1[4], long size_2[4]);
  double**   combineVectorCoords(double **vals_1, double **vals_2, long size_1[4] , long size_2[4]);
  
 
  //TODO: be more consistent with member vs non member variables 
 protected: // Configuration variables 
  std::string mask_rho_var_name;
  
  std::string lat_var_name;
  std::string lon_var_name;
  std::string lat_v_var_name;
  std::string lon_v_var_name;
  std::string lat_u_var_name;
  std::string lon_u_var_name;
  std::string angle_var_name;
  
  std::string time_var_name;
  std::string s_var_name;
  std::string bathy_var_name;
  // std::string scalar_output_var;
  std::string debug_name;

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
  double       m_east_value;
  double       m_north_value;
  //current depth level, and distances to nearest s_levels
  int          s_level;
  double       dist_sigma;
  double       dist_sp1;
  bool         above_s_level;
  
  //closest 4 eta/xi pairs(so eta[0] and xi[0] form one pair)and the respecitve distances to them
  int          eta_rho_index[4];
  int          xi_rho_index[4];  
  double       rho_dist[4];

  int          eta_east_index[4];
  int          xi_east_index[4];

  int          eta_north_index[4];
  int          xi_north_index[4];

  double       vec_dist[4];

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
  double****   rho_vals;
  double**     mask_rho;
  double**     lat;
  double**     lon;
  double**     meters_n;
  double**     meters_e;
  double**     bathy;
  double*      time;
  double*      s_values;

 
  double**     angle;

  double****   east_values;
  double****   north_values;

  double** vec_meters_e;
  double** vec_meters_n;
  int vec_size[4];
  
  friend bool CMOOSGeodesy::LocalGrid2LatLong(double dfEast, double dfNorth, double &dfLat, double &dfLon) ;

};

#endif




