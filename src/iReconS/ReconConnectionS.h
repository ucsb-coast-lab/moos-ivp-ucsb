#ifndef _RECON_CONNECTION_H_
#define _RECON_CONNECTION_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// Maximum rate to send RECON packets, so they're not dropped
#define MAX_RECON_UDP_XMIT_HZ 500

#define MAX_RECON_MSG_SIZE 1024

class ReconConnection
{
  
public:
  
  // Constructor, destructor
  ReconConnection();
  ~ReconConnection();
  
  // Setup (TODO - Serial setup..)
  int ConnectUDP(const char *remoteHost,
                 const char *remotePort,
                 const char *localPort);
  //int ConnectSerial(const char *device,
  //                  int baud);
  int Disconnect();
  
  // Input/Output
  int Send(const char *buf, size_t len, bool ackNeeded);
  int Recv(char *buf, size_t len);
  
private:
  
  // Network address query
  struct addrinfo *LookupAddrInfo(const char *node, const char *service);
  
  // UDP Network settings
  int reconSock;
  struct addrinfo *reconAddr;
  
  // Serial Settings (TODO)
  //FILE *reconSerial;
};

#endif
