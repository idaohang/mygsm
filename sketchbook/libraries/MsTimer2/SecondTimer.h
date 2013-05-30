#ifndef SECONDTIMER_H
#define SECONDTIMER_H

#include "MsTimer2.h"

#define SUBMAX 10

struct SubTimer
{
	int id;
	int sec;
	void (*func)(); 
    int count;
};

class SecondTimer
{
public:
	SecondTimer():count(0){}
     void set(int id, int seconds, void (*f)());
     //void stop(int id);
	 void stop();
     void start();
     void timerIsr();	
private:
	SubTimer subs[SUBMAX];
	int count;
};
extern SecondTimer stimer;
#endif