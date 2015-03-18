// This is anrp's SerMOOS class, modified here to be SerMR, 
// so as to avoid any conflicts.
//

#include "CSerMR.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include "byteorder.h"
#include "binary2hex.h"
#include <poll.h>
#include "sutil.h"

CSerMR::CSerMR(string serport, int baud)
{
  port = new CnSerialPort(serport);
  port->SetBaudRate(baud);

  running = false;
  pthread_create(&thr, NULL, &tramp, this);
}

CSerMR::~CSerMR()
{
  running = false;
  pthread_join(thr, NULL);
  delete port;
}

void CSerMR::BlockingWrite()
{
  port->NonBlockingWrite();
}

void CSerMR::AppendWriteQueue(char* buf, int len)
{
  port->AppendWriteQueue(buf, len);
}

void CSerMR::Thread()
{
  // From AIS script
  running = true;
  while (running) {
    port->BlockingRead();
    while(port->FindCharIndex('\r') != -1 ||
          port->FindCharIndex('\n') != -1) {
      int pos = port->FindCharIndex('\n');
      if(pos == -1) {
        pos = port->FindCharIndex('\r');
        if(pos == -1) {
          // huh?                    
          port->AllQueueFlush();
          continue;
        }
      }
      
      bom mem = port->Read(pos+1).c_str();
      strzero_space(mem.c());
      
      // Look for good message identifiers
      char *p = strchr(mem.c(), '$');
//      printf(p); // This prints the inbound message
      if(p == NULL) {
	//p = strchr(mem.c(), '!');
	//if(p == NULL) {
	//printf("Got an invalid message...\n");
	continue;
      	//}
      }
      
      if(cb) {
	cb(up, p);
      }
    }
  }
}
