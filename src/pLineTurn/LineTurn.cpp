/************************************************************/
/*    NAME: cmoran                                               */
/*    ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*    FILE: LineTurn.cpp                                         */
/*    DATE: 11 February 2019                                     */
/************************************************************/

#include <iterator>
#include <cmath>
#include "MBUtils.h"
#include "LineTurn.h"

// Struct and method should be identical in pLineFollow
struct Coordinate {
   double x,y;
};

struct Coordinate angle_transform(struct Coordinate c_1, double theta)
{
   struct Coordinate c_2 = c_1;
   c_2.x = c_1.x * cos(theta * PI / 180) - c_1.y * sin(theta * PI / 180);
   c_2.y = c_1.x * sin(theta * PI / 180) + c_1.y * cos(theta * PI / 180);

   return c_2;
}


using namespace std;

//---------------------------------------------------------
// Constructor

LineTurn::LineTurn()
{
	// m_nav_x,m_nav_y, and m_nav_heading are the real-time x,y, and vehicle-reference frame heading
	// data read in from MOOSDB
  m_nav_x = 0.0;
  m_nav_y = 0.0;
  m_nav_heading = 0.0;

	// m_point_string gets written to UPDATES_TURNING, which the Helm subscribes to, s.t. when in the turning
	// mode, the m_point_string is dynamically written by the behavior to indicate the next waypoint the
	// vehicle will attempt to reach
  m_point_string = "point = 100,0";
	// m_mode is a string representing the active mode in the behavioral mode tree, read in from MOOSDB
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

				 // Assigns MOOSDB variables to member variables
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
	// TO_DO: Add'l improvements for non-East/West lines can be added
	// TO_DO: There's a some asymmetry on the left/right turning behaviors, although not sure why. Both seems robust and work well,
	// so this should probably be considered low-priority

	double turn_radius = 20; // Looks like this should be > 5
	double sqrt_2 = 1.414; // square root of 2, defined as a constant for octagonal geometry
	double s = turn_radius/(1+sqrt_2); // equation for side-length of a regular octagon
	double dext = 1.5;

	double left_boundary = 49;
	double right_boundary = 151;
	double bottom_boundary = -100;
	double line_angle = 70;
	double theta = 90 - line_angle; // Note: defines the angle of the longlines; is hard-coded to avoid accidental run-time changes
	// double top_boundary = ; // Not being used at the moment

	cout << "m_mode = " << m_mode  << endl;
	cout << "(nav_x,nav_y) = (" << m_nav_x << ", " << m_nav_y << ")" << endl;

	// TURNING BEHAVIORS
	// Appx. octagonal end boundary with last point inside of LineFollow zone for behavior hand-off
	if (m_nav_x < right_boundary && m_nav_x > (right_boundary - left_boundary)/2 && m_nav_heading > 60 && m_nav_heading < 130) {
			struct Coordinate c1 = { dext * s , 0 };
			struct Coordinate c2 = { dext * (s + (.707 * s )) ,  -(.707 * s) };
			struct Coordinate c3 = { dext * (s + (.707 * s )) , - (s + (.707 * s)) };
			struct Coordinate c4 = {s , - (turn_radius - 1.5) };

			c1 = angle_transform(c1,theta);
			c2 = angle_transform(c2,theta);
			c3 = angle_transform(c3,theta);
			c4 = angle_transform(c4,theta);

			struct Coordinate c5 = {right_boundary - 10, m_nav_y };
			double dx = (m_nav_x + c4.x - c5.x) / cos(line_angle * PI / 180) ;
			cout << "c4.x = " << c4.x+ m_nav_x << ", right_boundary - 10 = " << right_boundary - 10 << " -> dx = " << dx << endl;
			c5.y = dx * tan(theta * PI/ 180);

				m_point_string = "points = "+to_string(m_nav_x + c1.x)+","+to_string(m_nav_y + c1.y) + ":"
				+ to_string(m_nav_x + c2.x ) + "," + to_string(m_nav_y + c2.y ) + ":"
				+ to_string(m_nav_x + c3.x ) + "," + to_string(m_nav_y + c3.y ) + ":"
				+ to_string(m_nav_x + c4.x ) + "," + to_string(m_nav_y + c4.y ) + ":"
				+ to_string( c5.x ) + "," + to_string(m_nav_y - c5.y ) + ":";
		cout << "Turning RIGHT" << endl;
	}

	if (m_nav_x > left_boundary && m_nav_x < (right_boundary - left_boundary)/2 && m_nav_heading > 240 && m_nav_heading < 300) {
		struct Coordinate c1 = { dext * s , 0 };
		struct Coordinate c2 = { dext * (s + (.707 * s )) ,  -(.707 * s) };
		struct Coordinate c3 = { dext * (s + (.707 * s )) , - (s + (.707 * s)) };
		struct Coordinate c4 = {s , - (turn_radius - 1.5) };

		c1 = angle_transform(c1, -theta);
		c2 = angle_transform(c2, -theta);
		c3 = angle_transform(c3, -theta);
		c4 = angle_transform(c4, -theta);

		struct Coordinate c5 = {left_boundary + 10, -turn_radius / abs (sin (theta) ) };
		double dx = (m_nav_x + c4.x - c5.x) / cos(line_angle * PI / 180) ;
		cout << "c4.x = " << c4.x+ m_nav_x << ", right_boundary - 10 = " << right_boundary - 10 << " -> dx = " << dx << endl;
		c5.y = dx * tan(theta * PI/ 180);


		m_point_string = "points = "+to_string(m_nav_x - c1.x)+","+to_string(m_nav_y + c1.y) + ":"
				+ to_string(m_nav_x - c2.x ) + "," + to_string(m_nav_y + c2.y ) + ":"
				+ to_string(m_nav_x - c3.x ) + "," + to_string(m_nav_y + c3.y ) + ":"
				+ to_string(m_nav_x - c4.x ) + "," + to_string(m_nav_y + c4.y ) + ":"
				+ to_string( c5.x  ) + "," + to_string(m_nav_y + c5.y ) + ":";

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
