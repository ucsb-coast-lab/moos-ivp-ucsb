#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include "ntpshm.h"

#define NTP_BASE 0x4e545030

ntpshm::ntpshm() {
  // Init
  shmunit = -1;
  shmid = 0;
  shmptr = NULL;
  
  // Verify timezone is GMT
  setenv("TZ", "GMT", 1);
}

ntpshm::~ntpshm() {
  // Detach from any existing segment
  if (shmptr != NULL)
  {
    shmdt(shmptr);
    shmunit = -1;
    shmid = 0;
    shmptr = NULL;
  }
}

int ntpshm::select(int unit)
{
  // Detach from any existing segment
  if (shmptr != NULL)
  {
    shmdt(shmptr);
    shmptr = NULL;
  }
  
  // Get NTP SHM segment
  shmid = shmget(NTP_BASE + unit, sizeof(struct shmTime), IPC_CREAT | 0644);
  if (shmid == -1) {
    // Failed
    perror("Cannot get SHM segment");
    return -1;
  }
  
  // Attach segment
  shmptr = (struct shmTime *)shmat(shmid, 0, 0);
  if (shmptr == (void*)-1)
  {
    // Failed
    perror("Cannot attach SHM segment");
    return -1;
  }
  
  // Init segment
  memset((void*)shmptr, 0, sizeof(struct shmTime));
  shmptr->mode = 1;
  shmptr->precision = -1;
  shmptr->nsamples = 3;
  shmptr->valid = 0;
  
  return 0;
}

int ntpshm::update(time_t secs, int usecs)
{
  struct timeval timenow;
  
  // Sanity check
  if (shmptr == NULL)
  {
    return -1;
  }
  
  // Time of update
  gettimeofday(&timenow, NULL);
  //printf("Called %s(%d,%d)\n  at (%d,%d)\n",
  //       __func__, secs, usecs, timenow.tv_sec, timenow.tv_usec);
  
  // Fill in SHM
  shmptr->count++;
  shmptr->clockTimeStampSec = (time_t)secs;
  shmptr->clockTimeStampUSec = (int)usecs;
  shmptr->receiveTimeStampSec = (time_t)timenow.tv_sec;
  shmptr->receiveTimeStampUSec = timenow.tv_usec;
  shmptr->count++;
  shmptr->valid = 1;
  
  return 0;
}
