#ifndef USR_MOOSAPP_HEADER
#define USR_MOOSAPP_HEADER
#include "NCData.h"
#include <string>
#include <string.h>//not totally sure why this needs to be here, but strcmp isn't found when it's not
#include "MOOS/libMOOS/MOOSLib.h" 
#include "MBUtils.h"
#include "AngleUtils.h"
#include <iostream>
#include <cmath>
#include <fstream>


class USR_MOOSApp : public CMOOSApp  
{
public:
  USR_MOOSApp();
  virtual ~USR_MOOSApp() {};

  bool Iterate();
  bool OnConnectToServer();
  bool OnDisconnectFromServer();
  bool OnStartUp();
  bool OnNewMail(MOOSMSG_LIST &NewMail);

 protected:
  void registerVariables();
  bool AdjustTime();
 protected: // Configuration variables
  std::string ncFileName;
  std::string varName;
  std::string m_rTime;

  std::string scalarOutputVar;

 protected: // State variables
  // x/y positions, dpeth, and altitude of current location 
  double       m_posx;
  double       m_posy;
  double       m_depth;
  double       m_floor_depth;
  double       m_altitude;    
  double       m_current_time;
  
  //the value representing a nonexistent value, replaced by mask_rho?
  double       bad_val;
  //geodesy class
  CMOOSGeodesy geodesy;
  
  NCData ncdata;

};

 


#endif




