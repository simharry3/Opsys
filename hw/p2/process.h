#ifndef __PROCESS_H_INCLUDED__
#define __PROCESS_H_INCLUDED__

#include <string>
#include <list>
#include <iostream>
#include <sstream>

using namespace std;

class process
{
public:
	process();
	process(string initializer);

	//SETTERS:
	void delayArrivalTime(int t);
	void delayArrivalTimeRunning(int t);
	void delayNextUsageTime(int t);

	//GETTERS:
	char getProcessID(){return processID;}
	int getMemSize(){return memSize;}
	int getNextArrivalTime(){return arrivalTimes.front();}
	int getNextUsageTime(){return usageTimes.front();}
	bool checkEmpty(){return arrivalTimes.empty();}

	//MISC:
	void printStatistics();
	void completeCycle();

private:
	char processID;
	int memSize;
	list<int> arrivalTimes;
	list<int> usageTimes;
};


#endif