/************************************************************/
/*    NAME: cmoran                                              */
/*    ORGN: MIT                                             */
/*    FILE: SAMSExecutive.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "SAMSExecutive.h"

using namespace std;

//---------------------------------------------------------
// Constructor

SAMSExecutive::SAMSExecutive()
{
  // m_nav_x,m_nav_y, and m_nav_heading are the real-time x,y, and vehicle-reference frame heading
  // data read in from MOOSDB
  m_nav_x = 0.0;
  m_nav_y = 0.0;
  m_nav_heading = 0.0;

  m_mode = "";

  m_point_string = "";
  m_iterator = 0;

  // m_farm actually needs to get declared in the header file, or else an error will occur

}

//---------------------------------------------------------
// Procedure: OnNewMail

bool SAMSExecutive::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key   = msg.GetKey();
    double dval = msg.GetDouble();
		string sval = msg.GetString();

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

bool SAMSExecutive::OnConnectToServer()
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

void SAMSExecutive::RegisterVariables()
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

bool SAMSExecutive::Iterate()
{
  // Using 'lib_mariner_sams' heavily here

  Coordinate a = {-10,-10};
  Coordinate b = {210,60};
  Coordinate c = {235,210};
  Coordinate d = {90,220};

  // 'box[]' needs to be composed of FOUR UNIQUE Coordinates
  Coordinate box[] = {d,a,b,c};
  Coordinate * box_error_boundary = create_error_boundary(box,20,20);

  // Displays the boolean value of each farm waypoint
  for (int j = 0; j < sizeof(m_farm)/sizeof(*m_farm); j++ ) {
    if (j == (sizeof(m_farm)/sizeof(*m_farm) - 1) ) {
      cout << m_farm[j].searched << endl;
    }
    else {
      cout << m_farm[j].searched << ",";
    }
  }

  // Writes LINE_THETA to MOOSDB for LineFollow
  double line_theta = get_theta({ m_farm[m_iterator*2], m_farm[m_iterator*2 + 1]  });
  Notify("LINE_THETA",line_theta);

  // If the current position is close to either the start or end point, mark that point as searched
  double length_to_start = get_length({ m_farm[m_iterator*2], {m_nav_x,m_nav_y}  });
  double length_to_end = get_length({ m_farm[m_iterator*2 + 1], {m_nav_x,m_nav_y}  });
  if(length_to_start < 3.0 ) {
    //cout << "Setting m_farm[" << m_iterator*2 << "].searched to true" << endl;
    m_farm[m_iterator*2].searched = true;
  }
  if(length_to_end < 20.0 ) {
    //cout << "Setting m_farm[" << (m_iterator*2 + 1) << "].searched to true" << endl;
    m_farm[m_iterator*2 + 1].searched = true;
  }

  // Keeps the AUV moving from point to point during PROCEEDING (if all are searched, RETURN)
  if ((m_iterator >= 0) && ( m_iterator < sizeof(m_farm)/sizeof(*m_farm)/2 ) ) {
    if (m_farm[m_iterator*2].searched == false) {
      m_point_string = "point = "+to_string(m_farm[m_iterator*2].x)+","+to_string(m_farm[m_iterator*2].y);
      Notify("UPDATES_PROCEEDING",m_point_string);
    }
    else {
      m_point_string = "point = "+to_string(m_farm[m_iterator*2 + 1].x)+","+to_string(m_farm[m_iterator*2 + 1].y);
      Notify("UPDATES_PROCEEDING",m_point_string);
    }
    //cout << "m_iterator = " << m_iterator << " and must be <= " << (sizeof(m_farm)/sizeof(*m_farm)-1) << endl;
  }
  else {
    cout << "All points have been searched, RETURNING" << endl;
    Notify("RETURN","true");
  }

  // If the 'start' point is searched, but the 'end' isn't, set FOLLOW to true
  if ( (m_farm[m_iterator*2].searched == true) && (m_farm[m_iterator*2 + 1].searched == false) ) {
    cout << "In nice flow, FOLLOW should be set as 'true' now, PROCEED to 'false' " << endl;
    Notify(m_outgoing_state,"true");
  }
  // Incremts iterator is both 'start' and 'end' have been searched
  if ( (m_farm[m_iterator*2].searched == true) && (m_farm[m_iterator*2 + 1].searched == true) ) {
    m_iterator++;
    Notify(m_outgoing_state,"false"); // Returns MODE = PROCEEDING
  }

  // If we're out of bounds, return to starting point
  /*if( check_bounds(box_error_boundary,{m_nav_x,m_nav_y}) == true ) {
    cout << "Vehicle has exceeded ERROR boundary: returning to home" << endl;
    Notify("RETURN","true");
  }*/

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool SAMSExecutive::OnStartUp()
{
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  m_MissionReader.GetConfiguration(GetAppName(), sParams);

  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine     = *p;
    string sVarName  = MOOSChomp(sLine, "=");
    sLine = stripBlankEnds(sLine);

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

    if(MOOSStrCmp(sVarName, "OUTGOING_STATE")) {
        if(!strContains(sLine, " "))
      m_outgoing_state = stripBlankEnds(sLine);
    }


  }

  RegisterVariables();
  return(true);
}
