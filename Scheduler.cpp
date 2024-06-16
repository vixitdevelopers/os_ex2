#include "Scheduler.h"



Scheduler::Scheduler (int quantum_time)
{
  threadList = std::vector<ThreadPtr> (MAX_THREAD_NUM);
  readyList = std::make_unique<std::list<std::shared_ptr<Thread>>> ();
  sa.sa_handler = &Scheduler::timer_handler;
  if (sigaction (SIGVTALRM, &sa, nullptr) < 0)
  {
    ERR_MSG_SYS(ERR_SIGEMPTYACTION);
    EXIT_WITH_FAILURE;
  }
  if (sigemptyset (&set) || sigaddset (&set, SIGVTALRM))
  {
    ERR_MSG_SYS(ERR_SIGEMPTYSET);
    EXIT_WITH_FAILURE;
  }
  timer.it_value.tv_sec = (long) (quantum_time / (int) MICROSECONDS_TO_SECONDS);
  timer.it_value.tv_usec = (long) (quantum_time % (int) MICROSECONDS_TO_SECONDS);
  timer.it_interval.tv_sec = (long) (quantum_time / (int) MICROSECONDS_TO_SECONDS);
  timer.it_interval.tv_usec = (long) (quantum_time % (int) MICROSECONDS_TO_SECONDS);
  threadList[0] = std::make_shared<Thread> (MAIN_THREAD_ID);
  threadList[0]->set_state (RUNNING);
  threadList[0]->set_quantum_counter (1);
  runningThread = threadList[MAIN_THREAD_ID];
  for (int i = 1; i < MAX_THREAD_NUM; i++)
  {
    threadList[i] = nullptr;
  }
  if (setitimer (ITIMER_VIRTUAL, &timer, nullptr))
  {
    ERR_MSG_SYS(ERR_SETITIMER);
    EXIT_WITH_FAILURE;
  }
}
void Scheduler::wake_up_threads ()
{
  for (int i = 0; i < MAX_THREAD_NUM; i++)
  {
    if (threadList[i] == nullptr)
    {
      continue;
    }
    if (threadList[i]->get_sleep_timer () == quantom_total)
    {
      threadList[i]->set_sleep_timer (AWAKE);
      if (threadList[i]->get_state () != BLOCKED)
      {
        readyList->push_back (threadList[i]);
      }
    }
  }
}
void Scheduler::nextThread ()
{

  int has_jumped = sigsetjmp(*runningThread->get_buf(), 1);
  if (has_jumped != 0)
  {
    return;
  }
  blockTimerSignal ();
  if (threadList[runningThread->get_id()] != nullptr
      && runningThread->get_state()!=
         BLOCKED
      && runningThread->get_sleep_timer() == AWAKE)
  {
    readyList->push_back (runningThread);
  }
  quantom_total += 1;
  wake_up_threads();
  runningThread->set_state ( runningThread->get_state() == RUNNING ? READY :
                             BLOCKED);
  runningThread = readyList->front ();
  readyList->pop_front ();
  runningThread->set_state ( RUNNING);
  runningThread->increment_quantom();
  if (setitimer (ITIMER_VIRTUAL, &timer, nullptr))
  {
    ERR_MSG_SYS(ERR_SETITIMER);
    EXIT_WITH_FAILURE;
  }
  unblockTimerSignal ();
  siglongjmp (*runningThread->get_buf(), 1);

}
int Scheduler::get_min_id ()
{
  for (int i = 0; i < MAX_THREAD_NUM; i++)
  {
    if (threadList[i] == nullptr)
    {
      return i;
    }
  }
  return NONE_AVAILBLE;
}