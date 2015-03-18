#ifndef __CAIS_h__
#define __CAIS_h__

#include <string>
#include <vector>
#include <map>
#include "MOOSLib.h"
#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"

using namespace std;


class CAIS :public CMOOSApp{
public:
  CAIS(string ais_host, int port, double lat_origin, double lon_origin);
  ~CAIS();

  /* Position in DD.DDDDDD */
  CMOOSGeodesy m_geodesy;
  double lat_dd;
  double long_ddd;
  long   userid;
  double cog;
  double sog;
  int hdg;
  double nav_x, nav_y;
  int nav_status_bit;
  string nav_status;
  bool postable;

  // list of known mmsi numbers
  // pointer to list of names (indexed the same as the mmsi)
  // list of names
  
  void SetCB(bool (*cb)(void *, std::string s), void *up) { this->cb = cb; this->up = up; }
  
private:
  // From anrp CiAISNMEA
  int pt;
  pthread_t thr;
  bool running;
  static void *tramp(void *a) { ((CAIS *)a)->Thread(); return NULL; }
  void Thread();
  
  bool (*cb)(void *, std::string s);
  void *up;
  


};

// desired info to save from AIS message 5, static info
struct contact {
  unsigned long   mmsi;                    // userid/MMSI
  string            name;         // name/ship name
  unsigned char   ship_type;         // ship_type/Type of ship and cargoe
  unsigned char   draught;                 // draught/maximum present static draught

};




#endif
