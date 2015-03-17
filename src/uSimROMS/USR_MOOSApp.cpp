/*===================================================================
File: USR_MOOSApp.cpp
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


#include "USR_MOOSApp.h"
#include <ctime>
using namespace std;

//------------------------------------------------------------------------
// Constructor

USR_MOOSApp::USR_MOOSApp() 
{
  // Initialize state vars
  m_posx     = 0;
  m_posy     = 0;
  m_depth    = 0;
  m_rTime = "2010-10-30 12:34:56";
  //default values for things
  scalar_output_var = "SCALAR_VALUE";
  east_output_var = "EAST_VALUE";
  north_output_var = "NORTH_VALUE";
  bad_val = -1;

}

//------------------------------------------------------------------------
// Procedure: OnNewMail
//      Note: reads messages from MOOSDB to update values

bool USR_MOOSApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
  CMOOSMsg Msg;
  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &Msg = *p;
    string key = Msg.m_sKey;
    double dval = Msg.m_dfVal;
    string sval = Msg.m_sVal;
    
    if(key == "NAV_X") m_posx = dval; 
    else if(key == "NAV_Y") m_posy = dval;
    else if(key == "NAV_DEPTH") m_depth = dval;
  }
    return(true);
}

//------------------------------------------------------------------------
// Procedure: OnStartUp
//      Note: initializes parameters based on what it finds in the moos file

bool USR_MOOSApp::OnStartUp()
{
  cout << "uSimROMS: SimROMS Starting" << endl;
  
  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);
    
  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine  = *p;
    string param  = stripBlankEnds(MOOSChomp(sLine, "="));
    string value  = stripBlankEnds(sLine);
    //double dval   = atof(value.c_str());

    param = toupper(param); 

    if(param == "NC_FILE_NAME"){  //required
       nc_file_name = value;
     }
    if(param == "OUTPUT_VARIABLE"){  //defaults to SCALAR_VALUE
       scalar_output_var = value;
       cout << "uSimROMS: publishing under name: " << scalar_output_var << endl;
    }
    if(param == "EAST_OUTPUT_VARIABLE"){
      east_output_var = value;
      cout << "uSimROMS: publishing east component under name " << east_output_var << endl;
    }
    if(param == "NORTH_OUTPUT_VARIABLE"){
      east_output_var = value;
      cout << "uSimROMS: publishing north component under name " << north_output_var << endl;
    }
    
    if(param == "SCALAR_VARIABLE"){  //e.g. salt or temperature
      var_name = value;
    }
    
    if(param == "VECTOR_VARIABLE_U"){
      vec_var_name[0] = value;
    }
    if(param == "VECTOR_VARIABLE_V"){
      vec_var_name[1] = value;
    }
    if(param == "VECTOR_VARIABLE_W"){
      vec_var_name[2] = value;
    }
    
    //this specifies the value that the ROMS file uses as a "bad" value, which for most applications is the
    //value the NC file uses to mean land , defaults to zero
    if(param == "BAD_VALUE"){
      bad_val = atof(value.c_str());
      cout << "uSimROMS: using " << bad_val << " as the bad value" << endl;
    }
    
  }
  // look for latitude, longitude global variables
  double lat_origin, long_origin;
  if(!m_MissionReader.GetValue("LatOrigin", lat_origin)){
    cout << "uSimROMS: LatOrigin not set in *.moos file." << endl;
    exit(0);
  }
  else if(!m_MissionReader.GetValue("LongOrigin", long_origin)){
    cout << "uSimROMS: LongOrigin not set in *.moos file" << endl;
    exit(0);
  }
  
  ncdata = NCData();
  ncdata.Initialise(lat_origin, long_origin, nc_file_name, var_name, vec_var_name, "uSimROMS");
  
  registerVariables();    
  
  
  cout << "uSimROMS: uSimROMS started" << endl;
  return(true);
}


//------------------------------------------------------------------------
// Procedure: OnConnectToServer
//      Note: 

bool USR_MOOSApp::OnConnectToServer()
{  
  cout << "uSimROMS: SimROMS connected" << endl; 
  return(true);
}

//------------------------------------------------------------------------
// Procedure: registerVariables()
//      Note: 

void USR_MOOSApp::registerVariables()
{
  m_Comms.Register("NAV_X", 0);
  m_Comms.Register("NAV_Y", 0);
  m_Comms.Register("NAV_DEPTH", 0);;
  m_Comms.Register("REMUS_TIME",0);
}  

//------------------------------------------------------------------------
// Procedure: OnDisconnectFromServer
//      Note: 

bool USR_MOOSApp::OnDisconnectFromServer()
{
  cout << "uSimROMS: new uSimROMS is leaving" << endl;
  return(true);
}

//------------------------------------------------------------------------
// Procedure: Iterate
//      notes: this bad boy calls everything else

bool USR_MOOSApp::Iterate()
{
  double  value, east_value, north_value; 
  if(!ncdata.Update(m_posx , m_posy, m_depth, m_current_time)){
    cout << "uSimROMS: something went wrong updating NCData, refusing to publish" << endl;
    return false;
  }
  
  value = ncdata.GetValue();
  east_value = ncdata.GetEastValue();
  north_value = ncdata.GetNorthValue();
  
  //if nothing has failed we can safely publish
  Notify(scalar_output_var.c_str(), value);
  cout << "uSimROMS: publishing value :" << value << endl;
  Notify(east_output_var.c_str(), east_value);
  cout << "uSimROMS: publishing east value of : " << east_value << endl;
  Notify(north_output_var.c_str(), north_value);
  cout << "uSimROMS: publishing north value of: " << north_value << endl;
  
  return(true);
}
    
bool USR_MOOSApp::AdjustTime()
{
  
  // NJN: 20141210: -This was getting the current time to index into
  //                the ROMS file, but it was getting the MOOStime, 
  //                not the time that's on REMUS. 
  //                -Also, without an argument, MOOSTime returns the 
  //                current time (in UNIX) times the warp.
  //                -I'll leave this here for debugging.
  //double current_time = MOOSTime(false);  
  //std::time_t t = static_cast<time_t>(current_time);
  //struct tm * moosTime;
  //moosTime = std::gmtime( &t);
  //cout << debugName<< "MOOS Time:  " << asctime(moosTime);

  std::time_t rt;
  std::time_t et;
  struct tm *remusTime;
  struct tm *epoch;
  double current_time;
  int year, month, day, hour, min, sec;

  rt = 1000;//have to initialize these to something 
  et = 10000;
  //cout << debugName<< "NCData::GetTimeInfo: setting unix epoch" << endl;
  epoch = gmtime(&et);
  epoch->tm_year = 1970 - 1900;
  epoch->tm_mon = 1 - 1;
  epoch->tm_mday = 1;
  epoch->tm_hour = 0;
  epoch->tm_min = 0;
  epoch->tm_sec = 0;

  //cout << debugName<< "NCData::GetTimeInfo: Adjusting REMUS time" << endl;
  remusTime = gmtime(&rt);
  sscanf(m_rTime.c_str(), "%d-%d-%d %d:%d:%d",&year,&month,&day,&hour,&min,&sec);
  remusTime->tm_hour = hour;
  remusTime->tm_min = min;
  remusTime->tm_sec = sec;
  remusTime->tm_mday = day;
  remusTime->tm_mon = month - 1;
  remusTime->tm_year = year - 1900;
   mktime(remusTime);
  //cout << debugName<< "NCData::GetTimeInfo: REMUS Time = " << asctime(remusTime);
  //cout << debugName<< "NCData::GetTimeInfo: UNIX epoch = " << asctime(epoch);

  // mktime converts to time_t
  m_current_time = difftime(mktime(remusTime), mktime(epoch));
  //cout << debugName<< "NCData::GetTimeInfo: current time = " << current_time << endl;
}
