#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include "portable.h"
#include "tcpsocket.h"

/* Create a TCP connection to host and port. Returns a file * descriptor on success, -1 on error. */
int tcpconnect (const char *host, int port) 
{
  struct hostent *h;
  struct sockaddr_in sa;
  int s;
  /* Get the address of the host at which to finger from the
   * hostname. */
  h = gethostbyname (host);
  if (!h || h->h_length != sizeof (struct in_addr)) {
     fprintf (stderr, "%s: no such host\n", host);
     return -1; 
  }

  /* Create a TCP socket. */
  s = socket (AF_INET, SOCK_STREAM, 0);

  /* Use bind to set an address and port number for our end of the 
   * finger TCP connection. */
  bzero (&sa, sizeof (sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (0); /* tells OS to choose a port */ 
  sa.sin_addr.s_addr = htonl (INADDR_ANY); /* tells OS to choose IP addr */ 
  if (bind (s, (struct sockaddr *) &sa, sizeof (sa)) < 0) {
    perror ("bind");
    close (s);
    return -1;
  }
  
  /* Now use h to set set the destination address. */ 
  sa.sin_port = htons (port);
  sa.sin_addr = *(struct in_addr *) h->h_addr;
  /* And connect to the server */
  if (connect (s, (struct sockaddr *) &sa, sizeof (sa)) < 0) {
    perror (host);
    close (s);
    return -1;
  }
 
  return s; 

}
