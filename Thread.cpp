//
// Created by dani on 6/6/24.
//

#include <cstring>
#include "Thread.h"

Thread::Thread (sigjmp_buf env, thread_entry_point entry_point, int id)
{
  _entry_point = entry_point;
  memcpy (_env, env, sizeof (sigjmp_buf));
  _tid = id;

}
int Thread::num_of_threads = 0;

State Thread::get_state ()
{
  return _state;
}
void Thread::set_state (State state)
{
  _state = state;
}
int Thread::get_id ()
{
  return _tid;
}
