#ifndef MinuteTimer_cpp
#define MinuteTimer_cpp
#include "SecondTimer.h"

SecondTimer stimer;

extern "C" void sc_func();
void SecondTimer::set(int id, int seconds, void (*f)())
{
	if (count < SUBMAX){
	  subs[count].id = id;
	  subs[count].sec = seconds;
	  subs[count].count = 0;
	  subs[count].func = f;
	  count++;
	}
}

void SecondTimer::stop()
{
  MsTimer2::stop();
}

void SecondTimer::start()
{
  MsTimer2::set(1000, sc_func); 
  MsTimer2::start();
}

void SecondTimer::timerIsr()
{
	for(int i=0;i<count;i++){
		subs[i].count++;
		if (subs[i].count >= subs[i].sec){
			subs[i].count=0;
			(*subs[i].func)();
		}
	}
}
void sc_func()
{
  stimer.timerIsr();
}
#endif