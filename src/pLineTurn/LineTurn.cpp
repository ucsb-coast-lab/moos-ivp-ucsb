/************************************************************/
/*    NAME: cmoran                                               */
/*    ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*    FILE: LineTurn.cpp                                         */
/*    DATE: 7 February 2019                                      */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "LineTurn.h"

using namespace std;

//---------------------------------------------------------
// Constructor

LineTurn::LineTurn()
{
  m_nav_x = 0.0;
  m_nav_y = 0.0;
  m_nav_heading = 0.0;
  m_point_string = "point = 100,0";
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool LineTurn::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    string key   = msg.GetKey();
    double dval = msg.GetDouble();

         if(key=="NAV_X") {
             m_nav_x = dval;
             //cout << "NAV_X = " << m_nav_x << endl;
         }

         if(key=="NAV_Y") {
             m_nav_y = dval;
						 //cout << "NAV_Y = " << m_nav_y << endl;
         }

         if(key=="NAV_HEADING") {
             m_nav_heading = dval;
						 //cout << "NAV_HEADING = " << m_nav_heading << endl;
         }

   }

   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool LineTurn::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);

   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void LineTurn::RegisterVariables()
{
	if(m_nav_x_received != "")
			 m_Comms.Register(m_nav_x_received, 0);
	if(m_nav_y_received != "")
			 m_Comms.Register(m_nav_y_received, 0);
	if(m_nav_heading_received != "")
			 m_Comms.Register(m_nav_heading_received, 0);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool LineTurn::Iterate()
{
	double vehicle_leader = 14.0;
	double turn_radius = 10; // Looks like this should be > 5

	// TURNING BEHAVIORS, which should be seperated to new function at some point
	// TO_DO: Declare boudaries as variables for easy modification
	if (m_nav_x > 151) {
		// TO_DO: Try making this a perfect circle radius using the radial or ellipse syntax (might not work, might need to do manually w/parameterization)
		// Note: doesn't look like you can choose couter/clockwise turn direction, so probably won't work
		// points = format=radial, label=foxtrot, x=0, y=40, radius=60, pts=6, snap=1
		// points = format=ellipse, label=golf, x=0, y=40, degs=45, pts=14, snap=1, major=100, minor=70
		// m_point_string = "points = format=radial, label=henry, x="+to_string(150)+",y="+to_string(-21.2)+", radius=20, pts=6, snap=0";
		// m_point_string = "points = format=ellipse, label=henry, x="+to_string(150)+",y="+to_string(-21.2)+", degs=0, pts=14, snap=7, major=20, minor=20";


		m_point_string = "point = "+to_string(m_nav_x-90)+","+to_string(m_nav_y-turn_radius);
		//m_point_string = "point = 170,-30:150,-40";
		cout << "Turning RIGHT" << endl;
	}

	if (m_nav_x < 49) {
		m_point_string = "point = "+to_string(m_nav_x+90)+","+to_string(m_nav_y-turn_radius);
		cout << "Turning LEFT" << endl;
	}

	if (m_nav_y < -100) {
		m_point_string = "point = "+to_string(0)+","+to_string(-105);
	}

  cout << "m_point_string: " << m_point_string << endl;
	//m_point_string = "point = 0,0";
  Notify(m_outgoing_var,m_point_string);

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool LineTurn::OnStartUp()
{
	list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  m_MissionReader.GetConfiguration(GetAppName(), sParams);

  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine     = *p;
    string sVarName  = MOOSChomp(sLine, "=");
    sLine = stripBlankEnds(sLine);

    if(MOOSStrCmp(sVarName, "OUTGOING_VAR")) {
      if(!strContains(sLine, " "))
	  m_outgoing_var = stripBlankEnds(sLine);
    }

		if(MOOSStrCmp(sVarName, "SET_TURN")) {
			if(!strContains(sLine, " "))
		m_change_state = stripBlankEnds(sLine);
		}

    if(MOOSStrCmp(sVarName, "NAV_X_RECEIVED")) {
      if(!strContains(sLine, " "))
    m_nav_x_received = stripBlankEnds(sLine);
    }

		if(MOOSStrCmp(sVarName, "NAV_Y_RECEIVED")) {
	     if(!strContains(sLine, " "))
	  m_nav_y_received = stripBlankEnds(sLine);
	  }

		if(MOOSStrCmp(sVarName, "NAV_HEADING_RECEIVED")) {
		    if(!strContains(sLine, " "))
		  m_nav_heading_received = stripBlankEnds(sLine);
		}

  }

  RegisterVariables();
  return(true);
}
