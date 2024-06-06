
#include <memory>
#include <queue>
#include <csignal>
#include "uthreads.h"
#include "Thread.h"

//queue of ready threads:
typedef std::shared_ptr<Thread> ThreadPtr;
typedef std::queue<ThreadPtr> ThreadQueue;

ThreadQueue readyThreads;
// list of ids:
std::vector<bool> thread_list;

// get the first available id:
int get_id ()
{
  for (int i = 0; i < thread_list.size (); i++)
  {
    if (!thread_list[i])
    {
      thread_list[i] = true;
      return i;
    }
  }
  return -1;
}

int uthread_init (int quantum_usecs)
{
  thread_list = std::vector<bool> (MAX_THREAD_NUM, false);
}

int uthread_spawn (thread_entry_point entry_point)
{
  // todo: add blocking of signals
  // check if entry point is not null:
  if (!entry_point)
  {
    // todo print shit
    return -1;
  }
  // check if exceedes max threads:
  if (Thread::num_of_threads >= MAX_THREAD_NUM)
  {
    // todo : print some shit
    return -1;
  }

  // get stack buffer:

  sigjmp_buf buf;
  sigsetjmp(buf, 1);
  sigemptyset (&(buf->__saved_mask));

  int id = get_id ();
  ThreadPtr thread_ptr = std::make_shared<Thread> (buf, entry_point, id);
  readyThreads.push (thread_ptr);
  //todo unblock signals
  return id;
}

