/************************************************************/
/*    NAME: cmoran                                               */
/*    ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*    FILE: LineTurn.cpp                                         */
/*    DATE: 11 February 2019                                     */
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
	m_mode = "";
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
		string sval  = msg.GetString();


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

				 if(key=="MODE") {
						m_mode = sval;
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
	if(m_mode_received != "")
	 		 m_Comms.Register(m_mode_received, 0);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool LineTurn::Iterate()
{
	// NOTE: The code for LineTurn and LineFollow behaviors is predicated on the assumption that the lines are located almost entirely East-West.
	// This could be refactored to accept a variation of theta, s.t. the start and end points of the lines would come into play, but the most
	// simple case of theta = 0 is implemented here. This could be a TO_DO in the future, but is being left alone for the moment while the general
	// behavior mechanics are being worked out.

	double turn_radius = 25; // Looks like this should be > 5
	double sqrt_2 = 1.414; // square root of 2, defined as a constant for octagonal geometry
	double s = turn_radius/(1+sqrt_2); // equation for side-length of a regular octagon

	double left_boundary = 49;
	double right_boundary = 151;
	double bottom_boundary = -100;
	// double top_boundary = ; // Not being used at the moment

	cout << "m_mode = " << m_mode  << endl;
	cout << "(nav_x,nav_y) = (" << m_nav_x << ", " << m_nav_y << ")" << endl;

	// TURNING BEHAVIORS
	// Octagonal end boundary with last point inside of LineFollow zone for behavior hand-off
	if (m_nav_x < right_boundary && m_nav_x > (right_boundary - left_boundary)/2) {
		m_point_string = "points = "+to_string(m_nav_x + s)+","+to_string(m_nav_y) + ":"
			+ to_string(m_nav_x + (s + (.707 * s ))) + "," + to_string(m_nav_y - (.707 * s) ) + ":"
			+ to_string(m_nav_x + (s + (.707 * s ))) + "," + to_string(m_nav_y - (s + (.707 * s)) ) + ":"
			+ to_string(m_nav_x + s) + "," + to_string(m_nav_y - turn_radius) + ":"
			+ to_string(right_boundary - 10) + "," + to_string(m_nav_y - turn_radius) + ":";
		cout << "Turning RIGHT" << endl;
	}

	if (m_nav_x > left_boundary && m_nav_x < (right_boundary - left_boundary)/2 ) {
		m_point_string = "points = "+to_string(m_nav_x-s)+","+to_string(m_nav_y) + ":"
			+ to_string(m_nav_x - (s + (.707 * s ))) + "," + to_string(m_nav_y - (.707 * s) ) + ":"
			+ to_string(m_nav_x - (s + (.707 * s ))) + "," + to_string(m_nav_y - (s + (.707 * s)) ) + ":"
			+ to_string(m_nav_x - s) + "," + to_string(m_nav_y - turn_radius) + ":"
			+ to_string(left_boundary + 10) + "," + to_string(m_nav_y - turn_radius) + ":";
		cout << "Turning LEFT" << endl;
	}

	if (m_nav_y < bottom_boundary) {
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

		if(MOOSStrCmp(sVarName, "MODE_RECEIVED")) {
				if(!strContains(sLine, " "))
			m_mode_received = stripBlankEnds(sLine);
		}

  }

  RegisterVariables();
  return(true);
}
