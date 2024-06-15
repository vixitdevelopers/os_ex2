#include "Scheduler.h"

Scheduler::Scheduler (int quantum_time)
{
  threadList = std::vector<ThreadPtr> (MAX_THREAD_NUM);
  readyList = std::make_unique<std::list<std::shared_ptr<Thread>>> ();
  sa.sa_handler = &Scheduler::timer_handler;
  if (sigaction (SIGVTALRM, &sa, nullptr) < 0)
  {
    std::cerr << "system error: couldn't apply sig action" << std::endl;
    exit (1);
  }
  if (sigemptyset (&set) || sigaddset (&set, SIGVTALRM))
  {
    std::cerr << "system error: couldn't empty set/add to set" << std::endl;
    exit (1);
  }
  timer.it_value.tv_sec = (long) (quantum_time / (int) MICROSECONDS_TO_SECONDS);
  timer.it_value.tv_usec = (long) (quantum_time % (int) MICROSECONDS_TO_SECONDS);
  timer.it_interval.tv_sec = (long) (quantum_time / (int) MICROSECONDS_TO_SECONDS);
  timer.it_interval.tv_usec = (long) (quantum_time % (int) MICROSECONDS_TO_SECONDS);
  threadList[0] = std::make_shared<Thread> (0);
  threadList[0]->set_state (RUNNING);
  threadList[0]->set_quantum_counter (1);
  runningThread = threadList[0];
  for (int i = 1; i < MAX_THREAD_NUM; i++)
  {
    threadList[i] = nullptr;
  }
  if (setitimer (ITIMER_VIRTUAL, &timer, nullptr))
  {
    std::cerr << "system error: couldn't set timer" << std::endl;
    exit (1);
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
    if (threadList[i]->get_sleep_timer () == quantum_clock)
    {
      std::cout << "waking up: " << i << std::endl;
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
  quantum_clock += 1;
  wake_up_threads();
  runningThread->set_state ( runningThread->get_state() == RUNNING ? READY :
                             BLOCKED);
  runningThread = readyList->front ();
  readyList->pop_front ();
  runningThread->set_state ( RUNNING);
  runningThread->increment_quantom();
  if (setitimer (ITIMER_VIRTUAL, &timer, nullptr))
  {
    std::cerr << "system error: couldn't set timer" << std::endl;
    exit (1);
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