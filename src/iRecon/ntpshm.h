#ifndef NTPSHM_H
#define NTPSHM_H

#include <time.h>

struct shmTime {
  /* 0 - if valid set
   *       use values, 
   *       clear valid
   * 1 - if valid set 
   *       if count before and after read of 
   *       values is equal,
   *         use values 
   *       clear valid
   */
  int    mode;
  int    count;
  time_t clockTimeStampSec;      /* external clock */
  int    clockTimeStampUSec;     /* external clock */
  time_t receiveTimeStampSec;    /* internal clock, when external value was received */
  int    receiveTimeStampUSec;   /* internal clock, when external value was received */
  int    leap;
  int    precision;
  int    nsamples;
  int    valid;
  int    dummy[10]; 
};

class ntpshm {

public:
  
  ntpshm();
  ~ntpshm();
  
  int select(int unit);
  int update(time_t secs, int usecs);
  
private:

  int shmunit;
  int shmid;
  struct shmTime *shmptr;
  
};

#endif
