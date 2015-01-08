//this is the header file


#ifndef PSM_MOOSAPP_HEADER
#define PSM_MOOSAPP_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <string>
#include "MBUtils.h"
#include "AngleUtils.h"
class PSM_MOOSApp : public CMOOSApp  
{
public:
  PSM_MOOSApp();
  virtual ~PSM_MOOSApp() {};

  bool Iterate();
  bool OnConnectToServer();
  bool OnDisconnectFromServer();
  bool OnStartUp();
  bool OnNewMail(MOOSMSG_LIST&);

  void registerVariables();
  

  std::string Dest;
  std::string Cmd;
  std::string PacketType;
  std::string AckTimeoutSec;
  std::string var1;
  std::string msg1;

};

std::string string_to_hex(const std::string&);


#endif
