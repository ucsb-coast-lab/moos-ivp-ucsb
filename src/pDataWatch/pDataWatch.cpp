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

/*=================================================================
Some conventions, based on x-positive landward and a negative salinity gradient.
LEG numbers increase landward (in ROMEO direction)/
ROMEO is red, right, returning up-river.
SDIR is the direction of search. 
   SDIR ==  1: ROMEO direction
   SDIR == -1: OSCAR direction 

SPOS resolves to -1 when the ROMEO target has been achieved, 
and to 1 when the OSCAR target has been acheived. Zero when between these
two targets.  


NJN:2014-12-08: Changed SPLUS/SMINUS configuration to SPOS, to be 
                  more flexible
NJN:2014-12-10: Corrected some hi/lo threshold confusion... 
NJN:2015-03-11: Changed ON_ROMEO as a boolean flag to SDIR = {-1, 1}, 
                  which when combied with SPOS = {-1, 0, 1},
                  gives an easy way to flag whether we should loiter 
                  at the end of a transect. 
		  Changed PAPA/ECHO to ROMEO/OSCAR

SG 1/10/15: TODO: refactor to include function pointers to a user defined function? maybe that's doing too much? 
===================================================================*/


#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "pDataWatch.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "CurrentField.h"

using namespace std;


//------------------------------------------------------
// Constructor 
//
DW_MOOSApp::DW_MOOSApp()
{
  count = 0;
  phase = 0;
  vehicle_depth = 0;
  
  romeo_target = 0;
  oscar_target = 0;
  
  SPOS = 0;
  SDIR = 1; // default start is romeo.
}


//-----------------------------------------------------
// OnNewMail
// notes: if a new SCALAR_VALUE has been published since 
//        the last check, set salinity to that value and
//        increase the count by one
//
bool DW_MOOSApp::OnNewMail(MOOSMSG_LIST &NewMail)
{

 CMOOSMsg Msg;
  
  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &Msg = *p;
    string key = Msg.m_sKey;
    double dval = Msg.m_dfVal;
    string sval = Msg.m_sVal;
    
    if (key == invar) {
      //   cout << "if_entered" << endl;
      scalar = dval;                     //I have no idea why, but it appears that ,Msg.m_dfVal is not 
      //scalar = atof(sval.c_str());           // working, it may be SCALAR_VALUE is in a format that the
                                         // CMOOSMsg doesn't like, but atof works fin
      // cout << "pDW: scalar = " << scalar << endl;
    }
    
    if (key == "NAV_DEPTH") {
      vehicle_depth = dval;    // atof(sval.c_str());
    }

    if (key == "TRACKING") {
      //      if (dval > 0)
      //	{
      //  SDIR = 1;
      //}
      //else
      //{
      //  SDIR = -1;
      //}
    }

  }   
  return true;
}


//----------------------------------------------------
// Pocedure: OnStartUp
//    notes: reads in the threshold variable

bool DW_MOOSApp::OnStartUp()
{
  cout << "DataWatch Starting" << endl;
 
 
  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);
    
  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine  = *p;
    string param  = stripBlankEnds(MOOSChomp(sLine, "="));
    string value  = stripBlankEnds(sLine);
   
    param = toupper(param);
 
    if(param == "ROMEO_TARGET"){
      romeo_target = atof(value.c_str());
      cout << "pDW: ROMEO_TARGET = " << romeo_target << endl;
    }
    if(param == "OSCAR_TARGET"){
      oscar_target = atof(value.c_str());
      cout << "pDW: OSCAR_TARGET =  " << oscar_target << endl;
    }
    if(param == "SCALAR_SOURCE"){
      invar = value;
      cout << "pDW: Monitoring the scalar values from: " << invar << endl;
    }
    if(param == "POINTS_AVERAGED"){
      points = atoi(value.c_str());
      cout << "pDW: averaging " << points << " points." << endl; 
    }
    if(param == "TIME_TO_EXCEED"){
      tte = atoi(value.c_str());
      cout << "pDW: Must exceed threshold for :  " << tte << " seconds." <<  endl;
    } 
  }
  
  // Initialize array for scalars
  sal = new double[points];
  for(int i = 0; i < points; i++){
    sal[i] = 0;
  }  
  
  if (romeo_target > oscar_target){
    dSdx = 1;
  } else {
    dSdx = -1;
  }
	       
   
  RegisterVariables();

  // Create output file for debugging
  std::time_t t = std::time(NULL);
  char YMDhms[31];
  std::strftime(YMDhms, sizeof(YMDhms), "pDataWatchLog_%Y%m%d-%H%M%S", std::gmtime(&t));
  reportname = YMDhms;
  
  // Should be checking for file-write here.  
  fout.open(reportname.c_str(), ios::out|ios::app);

  return (true);
}


//------------------------------------------------------------------------
// Procedure: OnConnectToServer
//     notes: just registers variables as of right now
//
bool DW_MOOSApp::OnConnectToServer()
{
  
  cout << "DataWatch connected" << endl;
  
  return(true);
}


//------------------------------------------------------------------------
// Procedure: registerVariables()
//     notes: subscribes to invar (which defaults to SCALAR_VALUE)
//
void DW_MOOSApp::RegisterVariables()
{
  m_Comms.Register(invar, 0);
  m_Comms.Register("NAV_DEPTH", 0);
  m_Comms.Register("TRACKING", 0);
}

//------------------------------------------------------------------------
// Procedure: OnDisconnectFromServer
bool DW_MOOSApp::OnDisconnectFromServer()
{
  fout.close();
  cout <<"DataWatch is leaving"<< endl;
  return(true);
}


//------------------------------------------------------------------------
// Procedure: Iterate
//     notes: after user specified number of points were had, takes a running average, determines which way to
//            flip the output varibale flag
//
bool DW_MOOSApp::Iterate()
{

  // We use the flag within iterate to notify the OnNewMail function 
  // to increase future flexibility for allowing other functions to 
  // change the tracking direction.
  if (count > points) {
    //	cout << "pDW: TRACKING" << endl;
    GradientTrackPlusMinus();
    if (SPOS * SDIR == -1){ 
      SDIR = SPOS;  // change direction.
      Notify("TRACKING",SDIR);
      }

    // Output to the log file
    std::time_t t = std::time(NULL);
    
    fout << t << ", ";
    fout << vehicle_depth << ", ";
    fout << sal[points - 1] << ", ";
    fout << avg << ", ";
    fout << phase << ", ";
    fout << SPOS << ", ";  
    fout << SDIR << endl;  
  } else {
	cout << "pDW: Too few points..." << endl;
    count ++;
  }

  return true;

}


//------------------------------------------------------------------------
//GradientTrackPlusMinus 
//  This reports the position the vehicle in the scalar field (SPOS).
//  When SPOS = 0, the vehicle is between the romeo and oscar targets.
//  SPOS = -1 vehicle is landward of romeo target.
//  SPOS = 1 is seaward of the oscar target.  
//
//  An iteration counter is implemented to ensure that target goals have
//  been met for a fixed period of time.
//
bool DW_MOOSApp::GradientTrackPlusMinus(){

   avg = RunningAverage(scalar);

   if(avg*dSdx < oscar_target*dSdx){
     phase = phase++;
     cout << "pDW: avg " << avg*dSdx << 
       " < OSCAR_TARGET (" << oscar_target*dSdx << 
       ") for " << phase << endl;

     if (phase > tte){
       SPOS = 1;
       return true;
     } 
   } else if (avg*dSdx > romeo_target*dSdx) {
     phase = phase++;
     cout << "pDW: avg " << avg*dSdx <<
       " > ROMEO_TARGET (" << romeo_target*dSdx << 
       ") for " << phase << endl;

     if (phase > tte){
       SPOS = -1;
       return true;
     }
   } else {
     cout << "pDW: SPOS == 0. Resetting counter" << endl;
     phase = 0;
     SPOS = 0;
     return false;
   }
   
}


//------------------------------------------------------------------------
//RunningAverage
//notes:  computes a running average. RunningAverage should return values of zero
//        until it has at least the number of points the user wants to average.
//
double DW_MOOSApp::RunningAverage(double scalar){      

  double sstar = 0;
  double total = 0;

  for (int n = 0; n < points - 1; n++){ 
    sal[n] = sal[n+1];
    total += sal[n];
  }
  sal[points-1] = scalar;  // The new point
  total += scalar;
  
  sstar = total/points;

  return sstar;
}
