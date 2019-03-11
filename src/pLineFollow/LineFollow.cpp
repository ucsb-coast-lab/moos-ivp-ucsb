/****************************************************************/
/*   NAME: cmoran                                               */
/*   ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*   FILE: LineFollow.cpp                                       */
/*   DATE: 13 February 2019                                     */
/****************************************************************/
// TO_DO: Lots of debugging code w/ 'cout' still exists. Will need to remove at
// some point.
#include <iterator>
#include <cmath>
#include "MBUtils.h"
#include "LineFollow.h"

// Struct and method should be identical in pLineTurn
// Creates a common data type for 2D coordinates
struct Coordinate {
   double x,y;
};

// Provides a convenient function for rotating a Coordinate struct based on a 2D angle transformation
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

LineFollow::LineFollow()
{
      // Initialise all State and non-string Configuration variables

			// m_nav_x,m_nav_y, and m_nav_heading are the real-time x,y, and vehicle-reference frame heading
			// data read in from MOOSDB
      m_nav_x = 0.0;
      m_nav_y = 0.0;
      m_nav_heading = 0.0;

			// m_distance is the distance of the max signal, as published to MOOS by pIncludeSampleData or pSimDistanceGenerator
			// Ideally, this is the distance from the sonar unit to the longline
			m_distance = 0.0;
			// m_point_string writes to UPDATES_LINE_FOLLOWING, which results in dynamically generated waypoints that the vehicle will
			// attempt to follow during the ACTIVE:SURVEYING:LINE_FOLLOWING mode
      m_point_string = "point = 100,0";
			// m_turn_iterator makes sure that the turning behavior occurs only one the first pass of the line has been made, such that
			// the LINE_TURN won't occur during the initial ACTIVE:APPROACHING mode, which technically otherwise meets the requirements
			// for a left-handed turn
			m_turn_iterator = 0;
			// m_mode subscribes to MOOSDB for the MODE, which is presented as a string that helps to govern when behaviors happen
			m_mode = "";
      // subscribed to the ideal angle that the long-line is following in the global reference frame
      m_line_theta = 0;

			// These variables all help form the moving average filter
		  m_distance_saved[5] = {};
		  m_distance_averaged = 0.0;
			m_iterations = 0;

}

//---------------------------------------------------------
// Procedure: OnNewMail

bool LineFollow::OnNewMail(MOOSMSG_LIST &NewMail)
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

				 if(key=="SIM_DISTANCE") {
             m_distance = dval;
             //cout << "SIM_DISTANCE = " << m_nav_x << endl;
         }

				 if(key=="MODE") {
						m_mode = sval;
				 }

         if(key=="LINE_THETA") {
             m_line_theta = dval;
             //cout << "LINE_THETA = " << m_line_theta << endl;
         }

   }

   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool LineFollow::OnConnectToServer()
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

void LineFollow::RegisterVariables()
{
  if(m_incoming_var != "")
      m_Comms.Register(m_incoming_var, 0);
	if(m_incoming_distance != "")
		   m_Comms.Register(m_incoming_distance, 0);
	if(m_nav_x_received != "")
			 m_Comms.Register(m_nav_x_received, 0);
	if(m_nav_y_received != "")
		   m_Comms.Register(m_nav_y_received, 0);
	if(m_nav_heading_received != "")
			 m_Comms.Register(m_nav_heading_received, 0);
	if(m_mode_received != "")
	 		 m_Comms.Register(m_mode_received, 0);
  if(m_line_theta_received != "")
   		 m_Comms.Register(m_line_theta_received, 0);
}


//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool LineFollow::Iterate()
{
	// TO_DO: Make it easier to distinguish recommended 'tuning knobs' for changing mission parameters
	// TO_DO: Use a bounding box rather than a left/right boundary line?

  //cout << "Iterate() was called" << endl;
	//cout << "m_distance = " << m_distance << endl;

	// Reads the size of the moving average filter as declared in the header file and LineFollow's constructor function
	int filter_size = (sizeof(m_distance_saved)/sizeof(*m_distance_saved));
	// Moving average filter of distance reports
	if (m_iterations < filter_size) {
		m_distance_saved[m_iterations] = m_distance;
	}
	else {
		for (int h = 0; h < filter_size; h++) {
			m_distance_saved[h] = m_distance_saved[h + 1];
		}
		m_distance_saved[filter_size] = m_distance;
	}
	m_iterations++; // Increases the filter iterator to account for when the number of iterations is less than filter size

	// Note: For debugging of moving average filter
	// for (int h = 0; h < filter_size; h++) { printf("m_distance_saved[%d] = %f\n",h,m_distance_saved[h]); }

	// Sums the values in the filter for use in the finding the average value
	double sum = 0.0;
	for (int i = 0; i < filter_size; i++) {
		sum = sum + m_distance_saved[i];
	}

	// Finds the avg value from the mvg avg filter, including when total number of values is less than the size
	// of the filter
	// TO_DO: Something in here causes a weird transition from CASE 1 to CASE 2,
	// where the sum value sits in the low 40's for a few iterations before
	// stabilizing in the 50s where it ideally should (low priority)
	double avg_dist;
	if (m_iterations <= 4) {
		avg_dist = sum/m_iterations;
		cout << avg_dist << " = " << sum << " / " << m_iterations << ": CASE 1" << endl;
	}
	else {
		avg_dist = sum/5.0;
		cout << avg_dist << " = " << sum << " / " << "5" << ": CASE 2" << endl;
	}

	// Note: it's important to remember that in the vehicle's reference frame, 0 degrees is aligned with North and 90 degrees with West
	double vehicle_leader = 25.0; // This is preliminary leading distance of the dynamically spawned waypoint in the x-direction
	double dist_ideal = 10.5; // Represents the ideal distance that the received signal should be at; will spawn waypoint such that the signal appears to be this far away

  // TO_DO: Need to check that this angle (based on the published LINE_THETA) is correct
  cout << "m_line_theta = " << m_line_theta << endl;
	double theta = m_line_theta;

	// TO_DO: Modify these conditionals so they don't use hard-coded angles, and eliminate any deadzones
	/*if (m_nav_heading > 10 && m_nav_heading < 170 ) {
		//cout << "Moving West, as NAV_HEADING = " << m_nav_heading << endl;
	  double x_init = vehicle_leader;
		double y_init = (dist_ideal - avg_dist);
		struct Coordinate c_orig = {x_init,y_init}; // creates a Coordinate using the vehicle leader and received signal distances
		struct Coordinate c_tfmd = angle_transform(c_orig,theta); // transforms the Coordinate location according to theta
		m_point_string = "point = "+to_string(m_nav_x + c_tfmd.x)+","+to_string(m_nav_y + c_tfmd.y);

	}
	else if (m_nav_heading > 190 && m_nav_heading < 350 ) {
		//cout << "Moving East, as NAV_HEADING = " << m_nav_heading << endl;
		double x_init = vehicle_leader;
		double y_init = (dist_ideal - avg_dist);
		struct Coordinate c_orig = {x_init,y_init};
		struct Coordinate c_tfmd = angle_transform(c_orig,theta);
		m_point_string = "point = "+to_string(m_nav_x - c_tfmd.x)+","+to_string(m_nav_y - c_tfmd.y);
	}
	else {
		cout << "Not in specified angle range!" << endl;
	}*/

  double x_init = vehicle_leader;
  double y_init = (dist_ideal - avg_dist);
  struct Coordinate c_orig = {x_init,y_init}; // creates a Coordinate using the vehicle leader and received signal distances
  struct Coordinate c_tfmd = angle_transform(c_orig,theta); // transforms the Coordinate location according to theta
  m_point_string = "point = "+to_string(m_nav_x + c_tfmd.x)+","+to_string(m_nav_y + c_tfmd.y);

	// Writes the m_point_string to UPDATES_LINE_FOLLOWING
  Notify(m_outgoing_point,m_point_string);

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool LineFollow::OnStartUp()
{
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  m_MissionReader.GetConfiguration(GetAppName(), sParams);

  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine     = *p;
    string sVarName  = MOOSChomp(sLine, "=");
    sLine = stripBlankEnds(sLine);

    if(MOOSStrCmp(sVarName, "OUTGOING_POINT")) {
      if(!strContains(sLine, " "))
	  m_outgoing_point = stripBlankEnds(sLine);
  }

  /*
		if(MOOSStrCmp(sVarName, "OUTGOING_STATE")) {
			if(!strContains(sLine, " "))
		m_outgoing_state = stripBlankEnds(sLine);
  }*/

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

		if(MOOSStrCmp(sVarName, "INCOMING_DISTANCE")) {
	     if(!strContains(sLine, " "))
	  m_incoming_distance = stripBlankEnds(sLine);
	     }

		if(MOOSStrCmp(sVarName, "MODE_RECEIVED")) {
				if(!strContains(sLine, " "))
			m_mode_received = stripBlankEnds(sLine);
		}

    if(MOOSStrCmp(sVarName, "LINE_THETA_RECEIVED")) {
				if(!strContains(sLine, " "))
			m_line_theta_received = stripBlankEnds(sLine);
		}

  }

  RegisterVariables();
  return(true);
}
