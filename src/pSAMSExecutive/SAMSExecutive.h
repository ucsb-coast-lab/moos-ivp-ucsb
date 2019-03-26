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

   // Need to have the farm declared here to allocate memory for the points that will be accessible
   // accross the function boundaries. The size of this array sets the maximum number of Coordinates
   // that can be specified in the OnStartUp() function
   Coordinate m_farm[64];

   std::string m_point_string;
   int m_iterator;
   double m_odometer;

};

#endif
