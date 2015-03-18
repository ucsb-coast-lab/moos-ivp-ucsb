#ifndef __CSerMR_h__
#define __CSerMR_h__

#include "CnSerialPort.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

class CSerMR {
 public:
  CSerMR(string port, int baud);
  ~CSerMR();

  void BlockingWrite();
  void AppendWriteQueue(char* buf, int len);

  void SetCB(bool (*cb)(void *, std::string s), void *up) { this->cb = cb; this->up = up; }


 private:
  CnSerialPort *port;
  pthread_t thr;
  bool running;
  static void *tramp(void *a) { ((CSerMR *)a)->Thread(); return NULL; }
  void Thread();

  bool (*cb)(void *, std::string s);
  void *up;
};

#endif
