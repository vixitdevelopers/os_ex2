
#include <memory>
#include <queue>
#include <csignal>
#include "uthreads.h"
#include "Thread.h"

//queue of ready threads:
typedef std::shared_ptr<Thread> ThreadPtr;
typedef std::deque<ThreadPtr> ThreadQueue;

static ThreadQueue readyThreads;
// list of ids:
static std::vector<ThreadPtr> thread_list;
static int running_thread_id = 0;
static int total_quantum = 0;

// get the first available id:
int get_min_id ()
{
  for (int i = 0; i < thread_list.size (); i++)
  {
    if (!thread_list[i])
    {
      return i;
    }
  }
  return -1;
}

void remove_from_queue (ThreadQueue queue, int tid)
{
  for (auto it = queue.begin (); it != queue.end (); ++it)
  {
    if ((*it)->get_id () == tid)
    {
      queue.erase (it);
    }
  }
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

  int id = get_min_id ();
  ThreadPtr thread_ptr = std::make_shared<Thread> (buf, entry_point, id);
  readyThreads.push_back (thread_ptr);
  thread_list[id] = thread_ptr;
  //todo unblock signals
  return id;
}

int uthread_terminate (int tid)
{
  // todo block signals

  if (!thread_list[tid])
  {
    //todo print stuff
    return -1;
  }
  if (tid == 0)
  {
    //todo main thread stuff
    exit (0);
  }

  if (running_thread_id == tid)
  {
    // need to terminate it
  }
  remove_from_queue (readyThreads, tid);
  //todo: remove also from blocked_queue
  return 0;
}

int uthread_get_total_quantums ()
{
  return total_quantum;
}

int uthread_get_quantums (int tid)
{
  if (!thread_list[tid])
  {
    //todo print stuff
    return -1;
  }
  return thread_list[tid]->get_total_quantoms ();
}
int uthread_get_tid ()
{
  return running_thread_id;
}