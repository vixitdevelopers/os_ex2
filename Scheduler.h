#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#include <iostream>
#include "uthreads.h"
#include <memory>
#include <queue>
#include "vector"
#include <list>
#include "memory"
#include "Thread.h"
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <cstdlib>

#define NONE_AVAILBLE (-1)
#define FAILED (-1)
#define MAIN_THREAD_ID 0
#define MIN_ID 0
#define ERR_SETITIMER "Error: setitimer failed"
#define ERR_SIGEMPTYACTION "Error: sigemptyaction failed"

#define MICROSECONDS_TO_SECONDS 1000000
typedef unsigned long address_t;

inline void blockTimerSignal ()
{
  sigset_t set;
  if (sigemptyset (&set) == FAILED)
  {
    ERR_MSG_SYS(ERR_SETITIMER);
    EXIT_WITH_FAILURE;
  }
  if (sigaddset (&set, SIGVTALRM) == FAILED)
  {
    ERR_MSG_SYS(ERR_SETITIMER);
    EXIT_WITH_FAILURE;
  }
  if (sigprocmask (SIG_BLOCK, &set, nullptr) == -1)
  {
    ERR_MSG_SYS(ERR_SETITIMER);
    EXIT_WITH_FAILURE;
  }
}

/**
 * @brief Unblocks the timer signal.
 */
inline void unblockTimerSignal ()
{
  sigset_t set;
  if (sigemptyset (&set) == FAILED)
  {
    ERR_MSG_SYS(ERR_SETITIMER);
    EXIT_WITH_FAILURE;
  }
  if (sigaddset (&set, SIGVTALRM) == FAILED)
  {
    ERR_MSG_SYS(ERR_SETITIMER);
    EXIT_WITH_FAILURE;
  }
  if (sigprocmask (SIG_UNBLOCK, &set, nullptr) == FAILED)
  {
    ERR_MSG_SYS(ERR_SETITIMER);
    EXIT_WITH_FAILURE;
  }
}
typedef std::shared_ptr<Thread> ThreadPtr;
class Scheduler;
inline std::unique_ptr<Scheduler> threadManager;
class Scheduler
{
 public :
  std::unique_ptr<std::list<ThreadPtr>> readyList;
  std::vector<ThreadPtr> threadList;
  std::shared_ptr<Thread> runningThread;
  int quantom_total = 1;
  struct itimerval timer;
  struct sigaction sa{};
  sigset_t set;

  Scheduler (int quantum_time);

  static void timer_handler (int sig)
  {
    threadManager->nextThread ();
  }
  void wake_up_threads ();

  void nextThread ();

  int get_min_id ();

};

#endif //_SCHEDULER_H_
