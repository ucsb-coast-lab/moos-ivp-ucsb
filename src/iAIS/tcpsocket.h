#ifndef __tcpsocket_h__
#define __tcpsocket_h__

#define bzero(ptr, size) memset (ptr, 0, size)

/* Prototypes */
int  tcpconnect(const char *host, int port);



#endif
