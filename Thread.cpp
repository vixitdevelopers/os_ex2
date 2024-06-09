//
// Created by dani on 6/6/24.
//

#include <cstring>
#include "Thread.h"
typedef unsigned long address_t;
#define JB_SP 4
#define JB_PC 5
#define STACK_SIZE 4096

address_t translate_address(address_t addr)
{
  address_t ret;
  asm volatile("xor    %%fs:0x30,%0\n"
               "rol    $0x11,%0\n"
      : "=g" (ret)
      : "0" (addr));
  return ret;
}

Thread::Thread (sigjmp_buf env, thread_entry_point entry_point, int id)
{
  if (env)
  {
    memcpy (_env, env, sizeof (sigjmp_buf));
  }
  _tid = id;
  if (entry_point)
  {
    if (_tid != 0)
    {
      (env->__jmpbuf)[JB_SP] = translate_address(
          (address_t) _stack + STACK_SIZE - sizeof(address_t));
      (env->__jmpbuf)[JB_PC] = translate_address((address_t)
          entry_point);
    }
    else
    {
      entry_point = nullptr;
    }
  }
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
sigjmp_buf &Thread::get_env ()
{
  return _env;
}
const char *Thread::get_stack () const
{
  return _stack;
}

