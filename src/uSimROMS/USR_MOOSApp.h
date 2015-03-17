/*===================================================================
File: USR_MOOSApp.h
Authors: Nick Nidzieko & Sean Gillen
Date: Jan-23-15
Origin: Horn Point Laboratory
Description: uSimROMS publishes simulated data from a given ROMS file
             for details on the implementation see the NCData class.

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
#ifndef USR_MOOSAPP_HEADER
#define USR_MOOSAPP_HEADER

#include "NCData.h"
#include "AngleUtils.h"
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
  bool AdjustTime();
 protected: // Configuration variables
  std::string nc_file_name;
  std::string var_name;
  std::string vec_var_name[3];  
  std::string m_rTime;

  std::string scalar_output_var;
  std::string east_output_var;
  std::string north_output_var;
 protected: // State variables
  // x/y positions, dpeth, and altitude of current location 
  double       m_posx;
  double       m_posy;
  double       m_depth;
  double       m_floor_depth;
  double       m_altitude;    
  double       m_current_time;
  
  //the value representing a nonexistent value, replaced by mask_rho?
  double       bad_val;
  //geodesy class
  CMOOSGeodesy geodesy;
  
  NCData ncdata;

};

 


#endif




