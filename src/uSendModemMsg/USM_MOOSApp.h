//this is the header file


#ifndef USM_MOOSAPP_HEADER
#define USM_MOOSAPP_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <string>
#include "MBUtils.h"
#include "AngleUtils.h"
class USM_MOOSApp : public CMOOSApp  
{
public:
  USM_MOOSApp();
  virtual ~USM_MOOSApp() {};

  bool Iterate();
  bool OnConnectToServer();
  bool OnDisconnectFromServer();
  bool OnStartUp();
  //bool OnNewMail(MOOSMSG_LIST &NewMail);

  std::string Dest;
  std::string Cmd;
  std::string PacketType;
  std::string AckTimeoutSec;

};

std::string string_to_hex(const std::string&);


#endif
