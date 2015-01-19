//new uSimROMS reads in data from a specified NCFile that contains ROMS output. right now the program can only read
//in scalar ROMS variables but this may be updated in future versions. the program first finds the 4 closest 
//points to the current lat/lon position, and using depth and altitude finds the closest 2 depth levels. it then 
// does an inverse weighted average of the values at all these points based on the distance from the current
// location. if there are time values in the future from the current time, the program will also take an inverse 
// weighted average of what the value would be at the two nearest time steps. if there is only one time step, or 
// if the last time step has been passed, the program simply uses the most recent time step as representing 
// the most accurate values
//
// NJN:2014-12-09: Added check for land values based on mask_rho
// NJN:2014-12-11: Fixd indexing to (x,y,z,t) = (i,j,k,n) and corrected distance calculations 
//                 (x pos was diffing against northing rather than easting)

#include <iostream>
#include <cmath>
#include <fstream>
#include "USR_MOOSApp.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include <string>
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
  time_step = 0;

  //defualt values for things
  scalarOutputVar = "SCALAR_VALUE";

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
//      Note: initializes paramters based on what it finds in the moos file

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
       ncFileName = value;
     }
    if(param == "OUTPUT_VARIABLE"){  //defaults to SCALAR_VALUE
       scalarOutputVar = value;
       cout << "uSimROMS: publishing under name: " << scalarOutputVar;
     }
  
    if(param == "SCALAR_VARIABLE"){  //e.g. salt or temprature
       varName = value;
     }
     //this specifies the value that the ROMS file uses as a "bad" value, which for most applications is the
     //value the NC file uses to mean land , defaults to zero
     if(param == "BAD_VALUE"){
       bad_val = atof(value.c_str());
       cout << "uSimROMS: using " << bad_val << " as the bad value" << endl;
     }
     
  }
  // look for latitude, longitude global variables
  double latOrigin, longOrigin;
  if(!m_MissionReader.GetValue("LatOrigin", latOrigin)){
    cout << "uSimROMS: LatOrigin not set in *.moos file." << endl;
    exit(0);
  }
  else if(!m_MissionReader.GetValue("LongOrigin", longOrigin)){
    cout << "uSimROMS: LongOrigin not set in *.moos file" << endl;
    exit(0);
  }
  
  ncdata = NCData();
  ncdata.Initialise(latOrigin, longOrigin, ncFileName, varName);

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
  m_Comms.Register("NAV_DEPTH", 0);
  m_Comms.Register("NAV_HEADING",0);
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
  double  value; 
  //cout << "USR: Getting time..." << endl;
  ncdata.GetTimeInfo(); 
  //cout << "USR: REMUS_TIME: " << m_rTime.c_str() << " at index=" << time_step << endl;
  
  // geodesy.LocalGrid2LatLong( m_posx,  m_posy, current_lat , current_lon );  //returns lat and lon
  //cout << "USR: converted lat/long to northing/easting" << endl;


  if(!ncdata.LatLontoIndex(m_posx, m_posy)){  //returns eta and xi, returns false if we're outside the ROMS grid, in which case 
    cout << "uSimROMS: no value found at current location" << endl;               //let the user know and don't publish
    return false;
  }

  m_altitude = floor_depth - m_depth;
  
  ncdata.GetBathy(floor_depth);

  ncdata.GetS_rho(m_depth, m_altitude);

  value = ncdata.GetValue();
  if(value == bad_val){  //if the value is good, go ahead and publish it
    cout << "uSimROMS: all local values are bad, refusing to publish new values" << endl;
    return false;
  }
  
  //if nothing has failed we can safely publish
  Notify(scalarOutputVar.c_str(), value);
  cout << "uSR: uSimROMS is publishing value :" << value << endl;
  
  return(true);
}
