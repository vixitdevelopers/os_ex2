//
// Created by dani on 6/6/24.
//

#ifndef _THREAD_H_
#define _THREAD_H_

#include <csetjmp>
#include "uthreads.h"
#define AWAKE (-1)

enum State
{
    READY,
    RUNNING,
    BLOCKED
};

class Thread
{
 private:
  int _id;
  State _state;
  int _sleep_timer;
  int _quantom_counter;
  sigjmp_buf _buf;
  char _stack[STACK_SIZE];
 public:
  explicit Thread (int id);
  void increment_quantom ()
  { _quantom_counter++; }
  int get_id () const
  { return _id; }
  void set_quantum_counter (int quantum_passed)
  { _quantom_counter = quantum_passed; }
  void set_state (State state)
  { _state = state; }
  void set_sleep_timer (int sleep_timer)
  { _sleep_timer = sleep_timer; }
  int get_sleep_timer () const
  { return _sleep_timer; }
  State get_state () const
  { return _state; }
  sigjmp_buf *get_buf ()
  { return &_buf; }
  char *get_stack ()
  { return _stack; }
  int get_quantom_counter () const
  { return _quantom_counter; }
};

#endif //_THREAD_H_
