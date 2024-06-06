//
// Created by dani on 6/6/24.
//

#ifndef _THREAD_H_
#define _THREAD_H_

#include <csetjmp>
#include "uthreads.h"
enum State
{
    READY, RUNNING, BLOCKED
};

class Thread
{
 private:
  int _tid;
  char _stack[STACK_SIZE];
  thread_entry_point _entry_point;
  int _quantum_usecs;

  State _state;
  sigjmp_buf _env;
 public:
  Thread (sigjmp_buf env, thread_entry_point entry_point, int id);
  static int num_of_threads;
  State get_state();
};

#endif //_THREAD_H_
