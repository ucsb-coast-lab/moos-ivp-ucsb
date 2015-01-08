#include <cstdio>
#include <unistd.h>
#include "ReconConnectionS.h"

ReconConnection::ReconConnection()
{
  // Initialize state
  reconSock = -1;
  reconAddr = NULL;
}

ReconConnection::~ReconConnection()
{
  Disconnect();
}

int ReconConnection::ConnectUDP(const char *remoteHost,
                                const char *remotePort,
                                const char *localPort)
{
  struct addrinfo *localAddr;
  
  // Make sure we're not connected before proceeding
  Disconnect();
  
  // Make a new socket
  reconSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (reconSock == -1)
  {
    perror("ERROR - Could not create socket");
    Disconnect();
    return -1;
  }
  
  // Bind to local port
  localAddr = LookupAddrInfo(NULL, localPort);
  if (bind(reconSock, localAddr->ai_addr, localAddr->ai_addrlen) == -1)
  {
    perror("ERROR - Could not bind to port");
    Disconnect();
    return -1;
  }
  freeaddrinfo(localAddr);
  
  // Lookup remote address
  reconAddr = LookupAddrInfo(remoteHost, remotePort);
  if (reconAddr == NULL)
  {
    perror("ERROR - Could not find RECON network address");
    Disconnect();
    return -1;
  }
  
  return 0;
}

int ReconConnection::Disconnect()
{
  // Shutdown network stuff as needed
  if (reconSock != -1)
  {
    close(reconSock);
    reconSock = -1;
  }
  if (reconAddr != NULL)
  {
    freeaddrinfo(reconAddr);
    reconAddr = NULL;
  }
  return 0;
}

// Returns - number of characters transmitted
int ReconConnection::Send(const char *buf, size_t len, bool ackNeeded)
{
  // TODO - Check whether UDP or Serial
  
  // Verify network settings
  if ((reconSock == -1) || (reconAddr == NULL))
  {
    return -1;
  }
  
  // Rate limit hack, cause REMUS drops UDP packets
  usleep(1000000 / MAX_RECON_UDP_XMIT_HZ);
  
  // Send data as UDP packet
  return (int)sendto(reconSock, buf, len, 0,
                     reconAddr->ai_addr, reconAddr->ai_addrlen);
}

// Returns - number of characters received
int ReconConnection::Recv(char *buf, size_t len)
{
  // TODO - Check whether UDP or Serial
  
  // Verify network settings
  if ((reconSock == -1) || (reconAddr == NULL))
  {
    return -1;
  }
  
  // Send data as UDP packet
  return (int)recv(reconSock, buf, len, 0);
}

struct addrinfo *ReconConnection::LookupAddrInfo(const char *node, const char *service)
{
  int rc;
  struct addrinfo hints, *addrPtr;
  
  // Set up hints to perform address lookup
  hints.ai_family = AF_INET;      // IPv4
  hints.ai_socktype = SOCK_DGRAM; // Datagram socket (UDP)
  hints.ai_flags = AI_PASSIVE;    // Return INADDR_ANY on node==NULL
  hints.ai_protocol = 0;          // Any protocol
  
  // Lookup
  rc = getaddrinfo(node, service, &hints, &addrPtr);
  if (rc != 0)
  {
    fprintf(stderr, "ERROR - getaddrinfo: %s\n", gai_strerror(rc));
    return NULL;
  }
  
  return addrPtr;
}

