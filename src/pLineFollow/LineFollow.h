/****************************************************************/
/*   NAME: cmoran                                               */
/*   ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*   FILE: LineFollow.h                                         */
/*   DATE: January 9, 2018                                      */
/****************************************************************/

#ifndef LineFollow_HEADER
#define LineFollow_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class LineFollow : public CMOOSApp
{
 public:
   LineFollow();
   //~LineFollow();

 protected: // Standard MOOSApp functions to overload
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

   void setOutgoingVar(std::string s) {m_outgoing_point=s;}
   void setIncomingVar(std::string s) {m_incoming_var=s;}
   //void setIncomingVar(std::string s) {m_incoming_distance=s;}

 protected:
   void RegisterVariables();

 protected: // Configuration variables
    std::string m_outgoing_point;
    std::string m_outgoing_state;

    std::string m_incoming_var;
    std::string m_nav_x_received;
    std::string m_nav_y_received;
    std::string m_nav_heading_received;
    std::string m_incoming_distance;
    std::string m_mode_received;
    std::string m_line_theta_received;


 protected: // State variables
     std::string m_mode;
     double m_nav_x;
     double m_nav_y;
     double m_nav_heading;
     double m_line_theta;
     double m_distance;
     std::string m_point_string;

     int m_turn_iterator;

     // Filter variables
     unsigned long int m_iterations;
     double m_distance_saved[5];
     double m_distance_averaged;

};

#endif
