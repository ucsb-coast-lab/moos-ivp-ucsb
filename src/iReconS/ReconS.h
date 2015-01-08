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

#include "ReconConnectionS.h"
#include "ntpshmS.h"
#include "AngleUtils.h"

#define MAX_RECON_MSG_SIZE 1024
#define MAX_RECON_FIELDS   32

// start Added by henrik for random heading
#define IM1 2147483563
#define IM2 2147483399
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 64
#define EPS 1.2e-15
// end

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
  
  // Added by henrik/mfallon for random heading
  double ran2(int & idum);

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
  
  // Internal state
  bool reconActive;
  bool wantReconActive;
  double vCourse;
  double vSpeed;
  double vDepth;
  
  bool recvNewCourse;
  bool recvNewSpeed;
  bool recvNewDepth;
  
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
  // Adde instead:
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

};

#endif
