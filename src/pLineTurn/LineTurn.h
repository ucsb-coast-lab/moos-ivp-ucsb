/************************************************************/
/*    NAME: cmoran                                               */
/*    ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*    FILE: LineTurn.h                                           */
/*    DATE: 7 February 2019                                      */
/************************************************************/

#ifndef LineTurn_HEADER
#define LineTurn_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class LineTurn : public CMOOSApp
{
 public:
   LineTurn();

 protected: // Standard MOOSApp functions to overload
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

   void setOutgoingVar(std::string s) {m_outgoing_var=s;}
   void setIncomingVar(std::string s) {m_incoming_var=s;}

 protected:
   void RegisterVariables();

 protected: // Configuration variables
   std::string m_incoming_var;
   std::string m_outgoing_var;
   std::string m_change_state;

   std::string m_nav_x_received;
   std::string m_nav_y_received;
   std::string m_nav_heading_received;
   std::string m_mode_received;

 protected: // State variables
   std::string m_mode;
   double m_nav_x;
   double m_nav_y;
   double m_nav_heading;
   std::string m_point_string;
};

#endif
