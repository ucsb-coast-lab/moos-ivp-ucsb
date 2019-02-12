/****************************************************************/
/*   NAME: cmoran                                               */
/*   ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*   FILE: SimDistanceGenerator_Info.cpp                        */
/*   DATE: December 13, 2018                                    */
/****************************************************************/

#include <iterator>
#include <random>
#include <cmath>
#include "MBUtils.h"
#include "SimDistanceGenerator.h"

using namespace std;

//---------------------------------------------------------
// Constructor

SimDistanceGenerator::SimDistanceGenerator()
{
		// Initialise all State and non-string Configuration variables
		m_range = 0.0;
    m_nav_x = 0.0;
    m_nav_y = -60.0;
    m_nav_heading = 0.0;
    m_iterator = 0;
    m_point_string = "point = 100,0";
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool SimDistanceGenerator::OnNewMail(MOOSMSG_LIST &NewMail)
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



   }

   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool SimDistanceGenerator::OnConnectToServer()
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

void SimDistanceGenerator::RegisterVariables()
{
    //if(m_incoming_var != "")
        //m_Comms.Register(m_incoming_var, 0);
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

bool SimDistanceGenerator::Iterate()
{
    int max = 10;
    int min = 1;
    random_device rd;     // only used once to initialise (seed) engine
    mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    uniform_int_distribution<int> uni(min,max); // guaranteed unbiased
    auto random_integer = uni(rng);
    double delta = random_integer/10.0;
    m_range = 10.0 + delta;

    double angle_limit = 10.0;
    double ideal_angle = 90.0;

    if (abs(ideal_angle - m_nav_heading) > angle_limit) {
        cout << "outside of necessary angle" << endl;

    }


		cout << "m_range: " << m_range << endl;
		cout << "(x,y,theta) = " << "( " << m_nav_x << " , " << m_nav_y << " ," << m_nav_heading << " )" << endl;


    Notify(m_outgoing_var,m_range);

	return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool SimDistanceGenerator::OnStartUp()
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
