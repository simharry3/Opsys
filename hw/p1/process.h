#ifndef __PROCESSS_H_INCLUDED__
#define __PROCESSS_H_INCLUDED__

#include <string>
#include <iterator>
#include <vector>
#include <iostream>
#include <list>
#include <sstream>

using namespace std;
class Process{
	string id;
	int initialArrivalTime;
	int cpuBurstTime;
	int numBursts;
	int ioTime;
	string status;

	vector<int> waitV;
	vector<int> turnaroundV;
	vector<int>	burstV;

	int waitArrival;
	int burstArrival;
	int burstTime;

public:
	int runtime;
	int progress;
	int burstProgress;
	int ioProgress;
	

	Process(string data);
	void printStatistics();
	bool runAndCheckComplete(); //Return true if process is complete
	bool runAndCheckIO();
	int getCPUBurstTime() const;
	int getArrivalTime();
	int getProgress(){return progress;}
	bool checkComplete(){return burstProgress == numBursts && progress == cpuBurstTime;}
	string checkStatus();
	void setStatus(string stat);
	void setIO(int t){ioTime = t;}
	int getIOTime();
	string getID();
	int burstRemaining(){return numBursts - burstProgress;}
	int timeRemaining(){return cpuBurstTime - progress;}

	int getBurstTime(){return cpuBurstTime * numBursts;}
	int getWaitTime();
	void setWaitTime(int t){waitArrival = t;}
	int getTurnaroundTime();
	int getNumBursts(){return numBursts;}
	void startWait(int t);
	void endWait(int t);
	void start(int t);
	void end(int t);
};
#endif
