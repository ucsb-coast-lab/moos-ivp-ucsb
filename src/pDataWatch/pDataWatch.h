/*===================================================================
File: pDataWatch.cpp
Authors: Nick Nidzieko & Sean Gillen
Date: Jan-23-15
Origin: Horn Point Laboratory
Description: pDataWatch will constantly watch for data coming in to 
             determine if it's over or under a certain threshold.
	     TODO: make a better description. 

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


#ifndef USC_MOOSAPP_HEADER
#define USC_MOOSAPP_HEADER
#include <iostream>
#include <fstream>
#include <string>
#include "MOOS/libMOOS/MOOSLib.h"

using namespace std;


class DW_MOOSApp : public CMOOSApp
{
 public:
  DW_MOOSApp();
  // DW_MOOSApp(int);
  virtual ~DW_MOOSApp() {};


  bool Iterate();
  bool OnConnectToServer();
  bool OnDisconnectFromServer();
  bool OnStartUp();
  bool OnNewMail(MOOSMSG_LIST &NewMail);


 protected:


  double RunningAverage(double);
  bool GradientTrack();
  bool GradientTrackPlusMinus();
  void RegisterVariables(); 
 
 protected:

  double oscar_target;
  double romeo_target;
  double dSdx;
  double phase; 
  int points;
  std::string invar;
  std::string outvar1;
  std::string outvar2;
  int tte; // Time to Exceed
  std::string reportname;  // for collecting variables
  ofstream fout;
 

 protected:  //state variables
  double scalar;
  int count; 
  double* sal;
  double avg;
  double old_depth; 
  double vehicle_depth;
  int   SPOS; // Position within the scalar field
  int   SDIR; // Direction of travel, postive up-river.
  std::string  TRACKING;
  


};
#endif
