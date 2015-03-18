
#include <iostream> 
#include "MOOS/libMOOS/MOOSLib.h"
#include <string>
#include "MBUtils.h"
#include "MOOSGeodesy.h"

class NZ_MOOSApp : public CMOOSApp  
{
public:
  NZ_MOOSApp();
  virtual ~NZ_MOOSApp() {};


  bool Iterate();
  bool OnConnectToServer();
  bool OnDisconnectFromServer();
  bool OnStartUp();
  bool OnNewMail(MOOSMSG_LIST &NewMail);

protected:
  void RegisterVariables();
  bool ReportZip();
  bool ReportUnZip();



protected:
  std::string  nodeReportVar;
  std::string  node_report;
  std::string  zipReportVar;
  std::string  zip_report;


  bool zipping;
  int count;
  int index;

  CMOOSGeodesy geodesy;
  double latOrigin;
  double lonOrigin;
};
