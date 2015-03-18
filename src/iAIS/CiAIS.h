// CiAIS.h: interface for the CiAIS class.                                          
////////////////////////////////////////////////  


#ifndef __CiAIS_h__
#define __CiAIS_h__

#include "MOOSLib.h"
#include "CAIS.h"  // this class has the thread to run the I/O



class CiAIS : public CMOOSApp
{
 public:
  CiAIS();
  virtual ~CiAIS();

  bool OnNewMail(MOOSMSG_LIST &NewMail);
  bool Iterate();
  bool OnConnectToServer();
  bool OnStartUp();

  CAIS *ais_stream;
 protected:
  // insert local vars here                                                                                 

  static bool tramp(void *arg, std::string s) {
    return ((CiAIS *)arg)->handle(s);
  }
  bool handle(std::string s);
};

#endif /* __CiAIS_h__ */

