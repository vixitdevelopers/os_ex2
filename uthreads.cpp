
#include <memory>
#include <queue>
#include <csignal>
#include <list>
#include <iostream>
#include "uthreads.h"
#include "Thread.h"
#include <iostream>
#include "uthreads.h"
#include <queue>
#include "vector"
#include <list>
#include "memory"
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <cstdlib>
#define JB_SP 6
#define JB_PC 7
//queue of ready threads:
typedef std::shared_ptr<Thread> ThreadPtr;
typedef std::list<std::shared_ptr<Thread>> ThreadQueue;

static ThreadQueue readyThreads;
// list of ids:
static std::vector<ThreadPtr> thread_list;
static int running_thread_id = 0;
static int total_quantum = 0;
static int quantum_len = 0;
static struct itimerval timer;
struct sigaction sa{};
sigset_t set;
// get the first available id:
void timer_signal_handler (int sig);
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
void block_signals ()
{
  sigset_t mask;
  if (sigfillset (&mask))
  {
    std::cerr << "system error: " << "sigfillset failed" << std::endl;
  }
  if (sigprocmask (SIG_SETMASK, &mask, NULL))
  {
    std::cerr << "system error: " << "sigprocmask failed" << std::endl;
  }
}

void unblock_signals ()
{
  sigset_t mask;
  if (sigemptyset (&mask))
  {
    std::cerr << "system error: " << "sigemptyset failed" << std::endl;
  }
  if (sigprocmask (SIG_SETMASK, &mask, NULL))
  {
    std::cerr << "system error: " << "sigprocmask failed" << std::endl;
  }
}
int start_timer()
{
  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = quantum_len;

  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = quantum_len;
  if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
  {
    //todo print error
    return -1;
  }
  return 0;
}


int uthread_init (int quantum_usecs)
{
  quantum_len = quantum_usecs;
  thread_list = std::vector<ThreadPtr> (MAX_THREAD_NUM);
  readyThreads = ThreadQueue ();
  sa.sa_handler = &timer_signal_handler;
  if (sigaction (SIGVTALRM, &sa, nullptr) < 0)
  {
    std::cerr << "system error: couldn't apply sig action" << std::endl;
    //todo is this what we want to print?
    exit (1);
  }
  // block the alarm signal because we dont have any threads yet:

//  if (sigemptyset (&set))
//  {
//    std::cerr << "system error: couldn't empty set to set" << std::endl;
//    exit (1);
//  }
//  if (sigaddset (&set, SIGVTALRM))
//  {
//    std::cerr << "system error: couldn't add to set" << std::endl;
//    exit (1);
//  }
  timer.it_value.tv_sec = (long) (quantum_usecs / (int) 10e6);
  timer.it_value.tv_usec = (long) (quantum_usecs % (int) 10e6);
  timer.it_interval.tv_sec = (long) (quantum_usecs / (int) 10e6);
  timer.it_interval.tv_usec = (long) (quantum_usecs % (int) 10e6);

  thread_list[0] = std::make_shared<Thread> (nullptr, nullptr, 0);
  thread_list[0]->set_state (RUNNING);
  running_thread_id = 0;
  if(setitimer(ITIMER_VIRTUAL, &timer, nullptr)){
    std::cerr  << "system error: couldn't set timer" << std::endl;
    exit(1);
  }
  return 0;
}

int uthread_spawn (thread_entry_point entry_point)
{
  // todo: add blocking of signals
  block_signals();
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
  if (sigemptyset (&(buf->__saved_mask)))
  {
    std::cerr << "system error: couldn't empty set" << std::endl;
    exit (1);
  }

  int id = get_min_id ();
  ThreadPtr thread_ptr = std::make_shared<Thread> (buf, entry_point, id);
  thread_ptr->set_state (READY);
  readyThreads.push_back (thread_ptr);
  thread_list[id] = thread_ptr;

  //todo unblock signals
  unblock_signals();
  return id;
}

void jmp_to_next_thread ()
{
  block_signals ();
  ThreadPtr next_thread = readyThreads.front ();
  readyThreads.pop_front ();
  next_thread->set_state (RUNNING);
  running_thread_id = next_thread->get_id ();
  // todo reset timer

  unblock_signals ();
  siglongjmp (next_thread->get_env (), 1);
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

  readyThreads.remove (thread_list[tid]);
  // todo remove stack from heap
  thread_list[tid] = nullptr;
  if (running_thread_id == tid)
  {
    jmp_to_next_thread ();
  }

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
  return 0;
//  return thread_list[tid]->get_total_quantoms ();
}
int uthread_get_tid ()
{
  return running_thread_id;
}

int uthread_resume (int tid)
{
  //todo block signals
  if (!thread_list[tid])
  {
    //todo print stuff
    return -1;
  }
  if (thread_list[tid]->get_state () == BLOCKED)
  {
    thread_list[tid]->set_state (READY);
    readyThreads.push_back (thread_list[tid]);
    //todo remove from blocked queue
  }
}

void timer_signal_handler (int sig)
{
  printf ("timer signal handler\n");
  switch (sig)
  {
    case SIGVTALRM:
      // remove the current thread from the queue and push it to the end of
      // the queue:
      block_signals ();
      sigsetjmp (thread_list[running_thread_id]->get_env (), 0);//todo why 0?
      //change status:
      thread_list[running_thread_id]->set_state (READY);
      readyThreads.push_back (thread_list[running_thread_id]);
      running_thread_id = readyThreads.front ()->get_id ();
      readyThreads.pop_front ();
      unblock_signals ();
      // jump to the next thread:
      siglongjmp (thread_list[running_thread_id]->get_env (), 1);
  }
}