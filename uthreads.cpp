#include <iostream>
#include "uthreads.h"
#include <memory>
#include <list>
#include "Scheduler.h"
#include <setjmp.h>
#include <signal.h>

#define JB_SP 6
#define JB_PC 7
#define STACK_SIZE 4096

address_t translate_address (address_t addr)
{
  address_t ret;
  asm volatile("xor    %%fs:0x30,%0\n"
               "rol    $0x11,%0\n"
      : "=g" (ret)
      : "0" (addr));
  return ret;
}

int uthread_init (int quantum_usecs)
{
  if (quantum_usecs <= 0)
  {
    ERR_MSG_UTHREADS(MSG_NEGATIVE_QUANTOM);
    return -1;
  }
  threadManager = std::make_unique<Scheduler> (quantum_usecs);
  return 0;
}

int uthread_spawn (thread_entry_point entry_point)
{
  blockTimerSignal ();
  int availble_id = threadManager->get_min_id ();
  if (entry_point == nullptr)
  {
    ERR_MSG_UTHREADS(ERR_ENTRY_POINT);
    return EXIT_WITH_FAILURE;
  }
  if (availble_id == NONE_AVAILBLE)
  {
    ERR_MSG_UTHREADS(ERR_MAX_THREADS_EXC);
    return EXIT_WITH_FAILURE;;
  }
  std::shared_ptr<Thread> newThread (new Thread (availble_id));
  sigsetjmp(*newThread->get_buf (), 1);
  ((*newThread->get_buf ())->__jmpbuf)[JB_SP] = translate_address (
      (address_t) newThread->get_stack () + STACK_SIZE - sizeof (address_t));
  ((*newThread->get_buf ())->__jmpbuf)[JB_PC] = translate_address (
      (address_t)
          entry_point);
  if (sigemptyset (&((*newThread->get_buf ())->__saved_mask)))
  {
    ERR_MSG_SYS(ERR_SIGEMPTYSET);
    EXIT_WITH_FAILURE;
  }
  threadManager->threadList[newThread->get_id ()] = newThread;
  threadManager->readyList->push_back (newThread);
  unblockTimerSignal ();
  return newThread->get_id ();
}

int uthread_terminate (int tid)
{
  blockTimerSignal ();
  if (tid < MIN_ID || tid >= MAX_THREAD_NUM)
  {
    ERR_MSG_UTHREADS(ERR_ID_NUM_ILLEGAL);
    unblockTimerSignal ();
    return EXIT_WITH_FAILURE;
  }
  if (tid == MAIN_THREAD_ID)
  {
    //this should kill all the threads:
    threadManager = nullptr;
    return EXIT_WITH_SUCCESS;
  }
  if (threadManager->threadList[tid] == nullptr)
  {
    ERR_MSG_UTHREADS(ERR_NO_THREAD_WITH_ID);
    return EXIT_WITH_FAILURE;
  }
  int cur_id = threadManager->runningThread->get_id ();
  threadManager->readyList->remove (threadManager->threadList[tid]);
  threadManager->threadList[tid] = nullptr;
  if (cur_id == tid)
  {
    threadManager->nextThread ();
  }
  unblockTimerSignal ();
  return EXIT_WITH_SUCCESS;
}

int uthread_block (int tid)
{
  if (tid < 0 || tid >= 100)
  {
    ERR_MSG_UTHREADS(ERR_ILLEGAL_ID);
    return -1;
  }
  if (tid == 0 || threadManager->threadList[tid] == nullptr)
  {
    ERR_MSG_UTHREADS(ERR_NO_THREAD_FOUND);
    return -1;
  }
  blockTimerSignal ();
  threadManager->threadList[tid]->set_state (BLOCKED);
  threadManager->readyList->remove (threadManager->threadList[tid]);
  if (tid == threadManager->runningThread->get_id ())
  {
    if (sigsetjmp(*threadManager->runningThread->get_buf(), 1) == 0)
    {
      threadManager->nextThread ();
    }
  }
  unblockTimerSignal ();
  return 0;
}

int uthread_resume (int tid)
{
  if (tid < 0 || tid >= 100)
  {
    ERR_MSG_UTHREADS(ERR_ILLEGAL_ID);
    return -1;
  }
  if (threadManager->threadList[tid] == nullptr)
  {
    ERR_MSG_UTHREADS(ERR_NO_THREAD_FOUND);
    return -1;
  }
  auto state = threadManager->threadList[tid]->get_state();
  if (state == RUNNING || state == READY)
  {
    return 0;
  }
  blockTimerSignal ();
  threadManager->threadList[tid]->set_state (READY);
  auto sleep_timer = threadManager->threadList[tid]->get_sleep_timer();
  if (sleep_timer == AWAKE)
  {
    threadManager->readyList->push_back (threadManager->threadList[tid]);
  }
  unblockTimerSignal ();
  return 0;
}

int uthread_sleep (int num_quantums)
{
  blockTimerSignal ();
  if (threadManager->runningThread->get_id () == MAIN_THREAD_ID)
  {
    ERR_MSG_UTHREADS(ERR_SLEEP_MAIN_THREAD);
    unblockTimerSignal ();
    return EXIT_WITH_FAILURE;
  }
  threadManager->runningThread->set_sleep_timer (
      uthread_get_total_quantums () + num_quantums);
  threadManager->nextThread ();
  unblockTimerSignal ();
  return 0;
}

int uthread_get_tid ()
{
  return threadManager->runningThread->get_id ();
}

int uthread_get_total_quantums ()
{
  return threadManager->quantom_total;
}

int uthread_get_quantums (int tid)
{
  if (tid < MIN_ID || tid >= MAX_THREAD_NUM)
  {
    ERR_MSG_UTHREADS(ERR_ID_NUM_ILLEGAL)
    return EXIT_WITH_FAILURE;
  }
  if (threadManager->threadList[tid] == nullptr)
  {
    ERR_MSG_UTHREADS(ERR_NO_THREAD_WITH_ID)
    return EXIT_WITH_FAILURE;
  }
  return (threadManager->threadList[tid])->get_quantom_counter ();
}
