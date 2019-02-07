/************************************************************/
/*    NAME: cmoran                                               */
/*    ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*    FILE: IncludeSampleData.h                                  */
/*    DATE: 6 February 2019                                      */
/************************************************************/

#ifndef IncludeSampleData_HEADER
#define IncludeSampleData_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class IncludeSampleData : public CMOOSApp
{
 public:
   IncludeSampleData();
   //~IncludeSampleData(); // No need for destructor?

 protected: // Standard MOOSApp functions to overload
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

   void setIncomingVar(std::string s) {m_incoming_var=s;}

 protected:
   void RegisterVariables();

 protected: // Configuration variables
     std::string m_incoming_var;

     std::string m_nav_x_received;
     std::string m_nav_y_received;
     std::string m_nav_heading_received;
     std::string m_mode_received;

     std::string m_input_filename;
     std::string m_output_filename;
     int m_colCount;
     int m_rowCount;

 protected: // State variables
     std::string m_mode;
     double m_nav_x;
     double m_nav_y;
     double m_nav_heading;

     unsigned long int m_iterations;

};

#endif
