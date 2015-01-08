// CiAIS.cpp: implementation of the CiAIS class.
////////////////////////////////////////////////////////

#include <iterator>
//#include "dtime.h" //what's this?
#include <signal.h>

#include "MOOSLib.h"
#include "CiAIS.h"

bool sigflag = false;

void sigh(int)
{
	sigflag = true;
}

CiAIS::CiAIS()
{
	// constructor
	signal(SIGINT, sigh);
	signal(SIGTERM, sigh);
}

CiAIS::~CiAIS()
{
	// destructor
}

bool CiAIS::OnNewMail(MOOSMSG_LIST &NewMail)
{
	MOOSMSG_LIST::iterator p;
	
	for(p = NewMail.begin(); p != NewMail.end(); p++) {
		CMOOSMsg &msg = *p;
	}

	NewMail.clear();
	
	return true;
}

bool CiAIS::OnConnectToServer()
{
  // register for variables here
  // possibly look at the mission file?
  // m_MissionReader.GetConfigurationParam("Name", <string>);
  // m_Comms.Register("VARNAME", <max frequency at which to get
  //                             updates, 0 = max>);
  // note, you cannot ask the server for anything in this function yet
  
  string hst; int pt;
  
  m_MissionReader.GetConfigurationParam("ais_host", hst);
  m_MissionReader.GetConfigurationParam("ais_port", pt);

 // look for latitude, longitude global variables
  double lat_origin, lon_origin;
  bool ok1 = m_MissionReader.GetValue("LatOrigin", lat_origin);
  bool ok2 = m_MissionReader.GetValue("LongOrigin", lon_origin);
 
  printf("\n\n  Attempting to connect to %s:%d \n\n",hst.c_str(),pt);

  ais_stream = new CAIS(hst, pt, lat_origin, lon_origin);
  //  ais_stream->SetCB(tramp, this);
  
  return true;
}

bool CiAIS::handle(string s)
{
  printf("S: %s\n", s.c_str());
  
  s += "\r\n";
  
  m_Comms.Notify("AIS_REPORT_RAW", s);

}

bool CiAIS::Iterate()
{
  // happens AppTick times per second

   if(ais_stream->postable) {
    printf("---> NAME=%ld,TYPE=ship,UTC_TIME=%f,X=%f,Y=%f,LAT=%f,LON=%f,SPD=%2.1f,HDG=%d,YAW=%3.1f,"
	    "DEPTH=0,LENGTH=100,MODE=%s\n\n", ais_stream->userid, MOOSTime(),
	    ais_stream->nav_x, ais_stream->nav_y, 
	    ais_stream->lat_dd, ais_stream->long_ddd, 
	    ais_stream->sog, ais_stream->hdg, ais_stream->cog, ais_stream->nav_status.c_str() );
    char bufff[256];
    sprintf(bufff, "NAME=%ld,TYPE=ship,UTC_TIME=%f,X=%f,Y=%f,LAT=%f,LON=%f,SPD=%2.1f,HDG=%d,YAW=%3.1f,"
	    "DEPTH=0,LENGTH=100,MODE=%s", ais_stream->userid, MOOSTime(),
	    ais_stream->nav_x, ais_stream->nav_y, 
	    ais_stream->lat_dd, ais_stream->long_ddd, 
	    ais_stream->sog, ais_stream->hdg, ais_stream->cog, ais_stream->nav_status.c_str() );

    std::string s = bufff;
    m_Comms.Notify("AIS_REPORT",s.c_str());
    m_Comms.Notify("NODE_REPORT",s.c_str());

    ais_stream->postable = false;
  }
  
  if(sigflag == true) { // time to exit
    delete ais_stream;
    exit(1);
	}



  
  return true;
}

bool CiAIS::OnStartUp()
{
	// happens after connection is completely usable
	// ... not when it *should* happen. oh well...
  
  return true;
}

