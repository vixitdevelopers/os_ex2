#include "Thread.h"

Thread::Thread (int id)
{
  _id = id;
  _state = READY;
  _quantom_counter = 0;
  _sleep_timer = AWAKE;
}
