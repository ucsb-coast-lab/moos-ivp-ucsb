/************************************************************/
/*    NAME: cmoran                                               */
/*    ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*    FILE: SAMSExecutive.h                                */
/*    DATE: 22 March 2019                                     */
/************************************************************/

#ifndef SAMSExecutive_HEADER
#define SAMSExecutive_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include "lib_mariner_sams.h"

class SAMSExecutive : public CMOOSApp
{
 public:
   SAMSExecutive();
   //~SAMSExecutive();

 protected: // Standard MOOSApp functions to overload
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected:
   void RegisterVariables();

 protected: // Configuration variables
   std::string m_outgoing_state;

   std::string m_nav_x_received;
   std::string m_nav_y_received;
   std::string m_nav_heading_received;
   std::string m_incoming_distance;
   std::string m_mode_received;

 protected: // State variables
   std::string m_mode;
   double m_nav_x;
   double m_nav_y;
   double m_nav_heading;

   // m_farm NEEDS to be declared here and not in the function constructor, or an error will occur
   // Farm variation #1 (~horizontal lines)
   //Coordinate m_farm[8] = { {25,-50,false}, {200,-50,false} , {200,-75,false}, {100,-100,false} , {100,-125,false}, {225,-175,false} , {225,-200,false}, {100,-200,false} };
   // Farm variation #2 (~vertical lines)
   Coordinate m_farm[8] = { {50,-50,false}, {50,-150,false} , {75,-175,false}, {75,-25,false} , {110,-25,false}, {125,-175,false} , {150,-175,false}, {175,-50,false} };

   // Basic coordinate for testing
   //Coordinate m_farm[2] = { {25,0,false}, {200,-50,false} };

   std::string m_point_string;
   int m_iterator;
   double m_odometer;

};

#endif
