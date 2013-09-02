#ifndef MinuteTimer_cpp
#define MinuteTimer_cpp
#include "MinuteTimer.h"

MinuteTimer mtimer;

extern "C" void c_func();
void MinuteTimer::set(int minutes, void (*f)())
{
  sec = minutes;
  thirtysec_total = sec*2;
  thirtysec_count = 0;
  func = f;
  MsTimer2::set(1000*30, c_func); 
}

void MinuteTimer::stop()
{
  MsTimer2::stop();
}

void MinuteTimer::start()
{
  thirtysec_count = 0;
  MsTimer2::start();
}

void MinuteTimer::timerIsr()
{
  thirtysec_count++;
  if (thirtysec_count >= thirtysec_total)
  {
    thirtysec_count=0;
    (*func)();
  }
}
void c_func()
{
  mtimer.timerIsr();
}
#endif