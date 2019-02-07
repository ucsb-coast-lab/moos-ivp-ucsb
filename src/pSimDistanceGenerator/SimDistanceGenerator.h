/****************************************************************/
/*   NAME: cmoran                                               */
/*   ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*   FILE: SimDistanceGenerator_Info.cpp                        */
/*   DATE: December 13, 2018                                    */
/****************************************************************/

#ifndef SimDistanceGenerator_HEADER
#define SimDistanceGenerator_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class SimDistanceGenerator : public CMOOSApp
{
 public:
   SimDistanceGenerator();
   //virtual ~SimDistanceGenerator();

   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

   void setOutgoingVar(std::string s) {m_outgoing_var=s;}
   // void setIncomingVar(std::string s) {m_incoming_var=s;}

 protected:
   void RegisterVariables();

 protected: // Configuration variables
   std::string m_outgoing_var;
   //std::string m_incoming_var;

   std::string m_nav_x_received;
   std::string m_nav_y_received;
   std::string m_nav_heading_received;

 protected: // State variables
  double m_range;
  double m_nav_x;
  double m_nav_y;
  double m_nav_heading;
  int m_iterator;
  std::string m_point_string;

};

#endif
