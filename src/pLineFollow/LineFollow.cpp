/****************************************************************/
/*   NAME: cmoran                                               */
/*   ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*   FILE: LineFollow.cpp                                       */
/*   DATE: 7 February 2019                                      */
/****************************************************************/
// TO_DO: Lots of debugging code w/ 'cout' still exists. Will need to remove at
// some point.
#include <iterator>
#include <random>
#include "MBUtils.h"
#include "LineFollow.h"

using namespace std;

//---------------------------------------------------------
// Constructor

LineFollow::LineFollow()
{
      // Initialise all State and non-string Configuration variables

      m_nav_x = 0.0;
      m_nav_y = 0.0;
      m_nav_heading = 0.0;
			m_distance = 0.0;
      m_iterator = 0;
      m_point_string = "point = 100,0";
			m_turn_iterator = 0;

			m_distance_received = 0.0;
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
    m_iterator++;
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

				 if(key=="SIM_DISTANCE") {
             m_distance = dval;
             //cout << "SIM_DISTANCE = " << m_nav_x << endl;
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
}


//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool LineFollow::Iterate()
{
  //cout << "Iterate() was called" << endl;
	//cout << "m_distance = " << m_distance << endl;


	// Moving average filter of distance reports
	if (m_iterations < 5) {
		m_distance_saved[m_iterations] = m_distance;
	}
	else {
		m_distance_saved[0] = m_distance_saved[1];
		m_distance_saved[1] = m_distance_saved[2];
		m_distance_saved[2] = m_distance_saved[3];
		m_distance_saved[3] = m_distance_saved[4];
		m_distance_saved[4] = m_distance_saved[5];
		m_distance_saved[5] = m_distance;
	}
	m_iterations++;

	double sum = 0.0;
	for (int i = 0; i < 5; i++) {
		sum = sum + m_distance_saved[i];
	}

	// TO_DO: Something in here causes a weird transition from CASE 1 to CASE 2,
	// where the sum value sits in the low 40's for a few iterations before
	// stabilizing in the 50s where it ideally should
	double avg_dist;
	if (m_iterations <= 4) {
		avg_dist = sum/m_iterations;
		cout << avg_dist << " = " << sum << " / " << m_iterations << ": CASE 1" << endl;
	}
	else {
		avg_dist = sum/5.0;
		cout << avg_dist << " = " << sum << " / " << "5" << ": CASE 2" << endl;
	}

	double vehicle_leader = 15.0;
	double turn_radius = 7.0;
	double dist_ideal = 10.5;

	if (m_nav_heading > 60 && m_nav_heading < 130) {
		//cout << "Moving West, as NAV_HEADING = " << m_nav_heading << endl;
  	m_point_string = "point = "+to_string(m_nav_x+vehicle_leader)+","+to_string(m_nav_y+(dist_ideal - avg_dist));
	}
	if (m_nav_heading > 240 && m_nav_heading < 300) {
		//cout << "Moving East, as NAV_HEADING = " << m_nav_heading << endl;
		m_point_string = "point = "+to_string(m_nav_x-vehicle_leader)+","+to_string(m_nav_y+(dist_ideal - avg_dist));
	}

	if (m_nav_x > 150 || m_nav_x < 50) {
		m_point_string = "Out of specified range";
		cout << "m_point_string: NADA "  << endl;
	}
	else {
		printf("(nav_x, nav_y) = (%f, %f)\n",m_nav_x,m_nav_y);
		cout << "m_point_string: " << m_point_string << endl;
	}
  //cout << "m_point_string: " << m_point_string << endl;
	//cout << "m_nav_x: " << m_nav_x << " m_nav_y: " << m_nav_y << endl;
  Notify(m_outgoing_var,m_point_string);


	// TURN = true/false LOGIC
	if (m_nav_x > 50) {
		m_turn_iterator++;
	}

	if ((m_nav_x > 150 || m_nav_x < 50) && m_turn_iterator > 1)  {
		Notify(m_sample_var,"true");
	}
	else { Notify(m_sample_var,"false"); }

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

    if(MOOSStrCmp(sVarName, "OUTGOING_VAR")) {
      if(!strContains(sLine, " "))
	  m_outgoing_var = stripBlankEnds(sLine);
    }

		if(MOOSStrCmp(sVarName, "OUTGOING_STATE")) {
			if(!strContains(sLine, " "))
		m_sample_var = stripBlankEnds(sLine);
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

		if(MOOSStrCmp(sVarName, "INCOMING_DISTANCE")) {
	     if(!strContains(sLine, " "))
	  m_incoming_distance = stripBlankEnds(sLine);
	     }

  }

  RegisterVariables();
  return(true);
}
