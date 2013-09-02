#ifndef MINUTETIMER_h
#define MINUTETIMER_h

#include "MsTimer2.h"

class MinuteTimer
{
public:
     void set(int minutes, void (*f)());
     void stop();
     void start();
     void timerIsr();	
private:
    void (*func)(); 
    int sec;
    int thirtysec_count;
    int thirtysec_total;
};
extern MinuteTimer mtimer;
#endif