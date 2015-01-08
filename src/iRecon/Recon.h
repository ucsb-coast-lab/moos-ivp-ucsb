#ifndef _RECON_H_
#define _RECON_H_

#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <MOOSApp.h>
#include <MOOSGeodesy.h>

#include "ReconConnection.h"
#include "ntpshm.h"
#include "AngleUtils.h"

#define MAX_RECON_MSG_SIZE 1024
#define MAX_RECON_FIELDS   32


class Recon : public CMOOSApp  
{
  
public:
  
  // Constructor, destructor
  Recon();
  virtual ~Recon() {};
  
  // MOOS connection handling
  bool OnStartUp();
  bool Iterate();
  bool OnConnectToServer();
  
  // Process inbound messages
  bool OnNewMail(MOOSMSG_LIST &NewMail);
  
  // Transmit UDP messages
  bool Transmit(const char *fmt, ...);
  
  // Receive inbound UDP messages
  void *ReconRecvLoop();
  
  // Added by mikerb to remove dependency on the moos-ivp
  // geometry library.
  double angle360(double);
private:
  
  // Handle all MOOS variable registration
  void DoRegistrations(double updateRate);
  
  // Engage/disengage RECON, post ready status
  void CheckReady();
  
  // Handle string booleans
  bool ParseBoolString(const char *buf);
  
  // Handle time from inbound state message
  void HandleStateTime(int hour, int min, int sec, int subsec);
  
  // REMUS RECON Message Parsers
  bool ParseMsgState(int numFields, char **fields);
  bool ParseMsgSet(int numFields, char **fields);
  // Extended parsers (njn)
  bool ParseMsgCTD(int numFields, char **fields);   
  bool ParseMsgModem(int numFields, char **fields);   


  // Internal state
  bool reconActive;
  bool wantReconActive;
  double vCourse;
  double vSpeed;
  double vDepth;
  
  bool recvNewCourse;
  bool recvNewSpeed;
  bool recvNewDepth;

  bool triangleMode;
  double tDepth;
  double tAltitude;
  double tRate;
  double tMaxD;
  
  bool allowKeepAlive;
  bool activateKeepAlive;
  bool rpmSpeedMode;
  
  // Ready signalling can be sent to any MOOS variable, and either by
  // using the typical true/false alphabetic strings, or by using a
  // 1/0 numeric value
  std::string readyVar;
  bool readyVarIsNumeric;
  
  // Receiver thread - Receives RECON messages and acts on them
  pthread_t reconRecvThread;
  
  // REMUS time
  bool remusTimeValid;
  struct tm remusTime;
  struct timeval remusTimeval;
  
  // For converting between local X/Y offsets and Lat/Lon
  CMOOSGeodesy m_Geodesy;
  
  // Connection to REMUS RECON
  ReconConnection recon;
  
  // NTP SHM driver
  ntpshm ntpRef;
  int ntpRefMaxSkew;

  // start Added by henrik for random heading
  // CRandom Random; // removed so as to remove dependency
  // Added instead:
  std::vector<int> iv;
  int iy;
  int idum2;
  double AM;
  int IMM1;
  int NDIV;
  double RNMX;
  // end
  bool random_hdg;
  double hdg_offset;
  double hdg_amplitude;
  double hdg_period;
  int hdg_seed;
  double last_time;

  // Timer for sending NODE_REPORTS
  time_t node_timer;
  double node_interval;

 
};

 std::string string_to_hex(const std::string&);
 std::string hex_to_string(std::string); 
#endif
