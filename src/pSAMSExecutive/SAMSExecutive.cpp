/************************************************************/
/*    NAME: cmoran                                              */
/*    ORGN: MIT                                             */
/*    FILE: SAMSExecutive.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <string>
#include <cmath>
#include "MBUtils.h"
#include "SAMSExecutive.h"

// Includes the Rust dependencies from parse_toml_rs directory
#include "parse_toml_rs.h"

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
  m_odometer = 0.0;

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

         // Reads the MOOSDB variables into member variables for this process (x/y position, heading, behavior mode)
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
  // Defines the boundaries of a bounding box in which the vehicle should operate
  Coordinate a = {-10,10};
  Coordinate b = {210,-60};
  Coordinate c = {235,-210};
  Coordinate d = {90,-220};

  // 'box[]' needs to be composed of FOUR UNIQUE Coordinates. They shouldn't necessarily need to be in the correct order
  // wrt quadrant I, II, III, IV, but this is probably best practice anyway
  Coordinate box[] = {d,a,b,c};
  // Using the specified bounding box to create an error boundary (box, m-meters, n-meters) outside of each vertex
  // If the vehicle moves beyond this bound, the RETURN behavior mode condition should be triggered
  // TO_DO: See if this can be done with the "pass by reference" method
  Coordinate * box_error_boundary = create_error_boundary(box,20,20);

  // Displays the boolean value of each farm waypoint, useful for debugging
  /*for (int j = 0; j < sizeof(m_farm)/sizeof(*m_farm); j++ ) {
    if (j == (sizeof(m_farm)/sizeof(*m_farm) - 1) ) {
      cout << m_farm[j].searched << endl;
    }
    else {
      cout << m_farm[j].searched << ",";
    }
  }*/

  // Writes LINE_THETA to MOOSDB for LineFollow. LineLength is defined for convenience keeping a leash during LineFollow
  double line_theta = get_theta({ m_farm[m_iterator*2], m_farm[m_iterator*2 + 1]  });
  double line_length = get_length({ m_farm[m_iterator*2], m_farm[m_iterator*2 + 1]  });
  Notify("LINE_THETA",line_theta);

  // If the current position is close to either the start or end point, mark that point as searched
  // Also mark the endpoint as searched if we get to the end of our 'leash', i.e. the distance between the start and end points,
  // plus a certain percentage has been exceeded
  double length_to_start = get_length({ m_farm[m_iterator*2], {m_nav_x,m_nav_y}  });
  double length_to_end = get_length({ m_farm[m_iterator*2 + 1], {m_nav_x,m_nav_y}  });
  if( (length_to_start < 3.0) && (m_farm[m_iterator*2].searched != true) ) {
    //cout << "Setting m_farm[" << m_iterator*2 << "].searched to true" << endl;
    m_farm[m_iterator*2].searched = true;
  }
  else if( (length_to_end < 5.0) && (    m_farm[m_iterator*2 + 1].searched != true)  ) {
    //cout << "Setting m_farm[" << (m_iterator*2 + 1) << "].searched to true" << endl;
    m_farm[m_iterator*2 + 1].searched = true;
  }
  else if ( (get_length({ m_farm[m_iterator*2], {m_nav_x,m_nav_y} }) > line_length*1.05) && (m_farm[m_iterator*2 + 1].searched != true) ) {
    //cout << "Exceeded maximum line length w/o hitting point, so assuming passed end and marking as true"  << endl;
    //cout << "M_ITERATOR = " << m_iterator << endl;
    //cout << "Setting m_farm[" << (m_iterator*2 + 1) << "].searched to true" << endl;
    m_farm[m_iterator*2 + 1].searched = true;
  }

  // Building a set of 3 'primer' points before actually hitting the approach
  double pl_1 = 15.0;
  double pl_2 = 25; // pl_1 * 1.666
  double pl_3 = 35.0; // pl_1 * 2.333
  double primer_angle = line_theta + 180;
  if (primer_angle > 360) {
    primer_angle = primer_angle - 360;
  }
  double pa_2 = 15;
  double pa_3 = 30;
  string primer_point_1 = to_string(m_farm[m_iterator*2].x + cos(primer_angle * PI / 180) * pl_1 )+","+to_string(m_farm[m_iterator*2].y + sin(primer_angle * PI / 180) * pl_1);
  string primer_point_2 = to_string(m_farm[m_iterator*2].x + cos((primer_angle + pa_2) * PI / 180) * pl_2 )+","+to_string(m_farm[m_iterator*2].y + sin((primer_angle + pa_2) * PI / 180) * pl_2);
  string primer_point_3 = to_string(m_farm[m_iterator*2].x + cos((primer_angle + pa_3) * PI / 180) * pl_3 )+","+to_string(m_farm[m_iterator*2].y + sin((primer_angle + pa_3) * PI / 180) * pl_3);
  string primer_point_string = primer_point_3+":"+primer_point_2+":"+primer_point_1+":";

  // Keeps the AUV moving from point to point during PROCEEDING (if all are searched, RETURN)
  // Writes that value to UPDATES_PROCEEDING, which the waypoint behavior PROCEEDING is subscribed to
  if ((m_iterator >= 0) && ( m_iterator < sizeof(m_farm)/sizeof(*m_farm)/2 ) ) {
    if (m_farm[m_iterator*2].searched == false) {
      string waypoint_string = to_string(m_farm[m_iterator*2].x)+","+to_string(m_farm[m_iterator*2].y);
      //m_point_string = "point = "+to_string(m_farm[m_iterator*2].x)+","+to_string(m_farm[m_iterator*2].y);
      m_point_string = "points = "+primer_point_string+waypoint_string;
      Notify("UPDATES_PROCEEDING",m_point_string);
    }
    else {
      string waypoint_string = to_string(m_farm[m_iterator*2 + 1].x)+","+to_string(m_farm[m_iterator*2 + 1].y);
      m_point_string = "points = "+waypoint_string;
      Notify("UPDATES_PROCEEDING",m_point_string);
    }
    //cout << "m_iterator = " << m_iterator << " and must be <= " << (sizeof(m_farm)/sizeof(*m_farm)-1) << endl;
  }
  else {
    cout << "All points have been searched, RETURNING" << endl;
    Notify("RETURN","true");
  }

  // If the 'start' point is searched, but the 'end' isn't, set FOLLOW to true, and engage in LINE_FOLLOWING behavior
  if ( (m_farm[m_iterator*2].searched == true) && (m_farm[m_iterator*2 + 1].searched == false) ) {
    Notify(m_outgoing_state,"true");
  }
  // Incremts iterator is both 'start' and 'end' have been searched
  // TO_DO: This can probably be combined into a single statement
  if ( (m_farm[m_iterator*2].searched == true) && (m_farm[m_iterator*2 + 1].searched == true) ) {
    cout << "Hit both points, so should be switching back to MODE = PROCEEDING " << endl;
    m_iterator++;
    Notify(m_outgoing_state,"false");
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
  cout << "From moos-ivp-ucsb in the home directory!" << endl;

  // Note: # of Coordinates in the farm can not exceed the size of the statically declared array in the header file
  // In this case, can not exceed 64 points (32 lines)

  // Farm variation #1 (~horizontal lines)
  //Coordinate m_farm[8] = { {25,-50,false}, {200,-50,false} , {200,-75,false}, {100,-100,false} , {100,-125,false}, {225,-175,false} , {225,-200,false}, {100,-200,false} };
  // Farm variation #2 (~vertical lines)
  //Coordinate m_points[] = { {50,-50,false}, {50,-150,false} , {75,-175,false}, {75,-25,false} , {110,-25,false}, {125,-175,false} , {150,-175,false}, {175,-50,false} , {150,-10,false}, {100,0,false} };

  string filename = "sams_config.toml";
  // Get the number of farm points, from a Rust library function (tests implemented)
  int task_num = get_number_of_tasks(filename.c_str());
  // For each point in the task list, s
  for (int i = 0; i < task_num; i++ ) {
    // From a Rust function, return the GeoCoor of the waypoint that has the same label as the
    // the first task in the toml file's 'task' array
    GeoCoor toml_point = return_task_position(filename.c_str(),i);
    //cout << toml_point.x << ", " << toml_point.y << endl;
    m_farm[i] = {toml_point.lat,toml_point.lon,false};
    //cout << "m_farm[" << i << "] = [" << m_farm[i].x << ", " << m_farm[i].y << "]" << endl;
  }

  GeoCoor start_point = return_start_point(filename.c_str());
  Coordinate start_coor = {start_point.lat,start_point.lon,false};
  cout << "start_coor[x,y] = [" << start_coor.x << ", " << start_coor.y << "]" << endl;


  // Notifying VIEW_SEGLIST with a list of points pulled from the m_farm Coordinates and
  // will plot a path, in order of appearance, between all of those points
  // TO_DO: This isn't done in the recommended way (see MOOS docs "Serializing Geometric Objects for pMarineViewer Consumption")
  //int task_num = sizeof(m_farm)/sizeof(*m_farm);
  std::string point_list = "pts={";
  for (int h = 0; h < task_num; h++) {
    if (h == (task_num - 1 ) ) {
      point_list = point_list+to_string(m_farm[h].x)+","+to_string(m_farm[h].y);
    }
    else {
      point_list = point_list+to_string(m_farm[h].x)+","+to_string(m_farm[h].y)+":";
    }
  }
  point_list = point_list+"}";
  //cout << "point_list = " << point_list << endl;
  std::string pl_configs = "edge_color=white,vertex_color=white,vertex_size=10,edge_size=1";
  Notify("VIEW_SEGLIST",point_list+","+pl_configs);

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
