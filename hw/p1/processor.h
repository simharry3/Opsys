//Written by Clayton Rayment
//Defines Processor class and helper functions which will allow for better extendability to future projects
#ifndef __PROCESSOR_H_INCLUDED__
#define __PROCESSOR_H_INCLUDED__

#include "process.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <iomanip>

using namespace std;

class Core{
	int coreID;
	int timeSlice;
	int timeSliceTracker;
	int runningTime;
	int t_l;
	int t_u;
	int cycles;
	vector<string> messages;
	Process* loadQueue;
	Process* unloadQueue;
	Process* activeProcess;
	string status;

	int contextSwitches;
	int preemptions;
public:
	Core(int cID, int tL, int tU, int tS);
	~Core();
	void setActiveProcess(Process* proc);
	Process* getActiveProcess();
	Process* execute(bool processWaiting);
	void unloadProcess();
	Process* getLoadQueue();
	Process* getUnloadQueue();
	void loadProcess(Process* proc);
	int executeContextSwitch();
	int getSliceProgress(){return timeSliceTracker;}
	void setSlice(int val){timeSliceTracker = val;}
	Process* checkWaitCondition();
	string getStatus();
	vector<string> getMessages();
	void setCycles(int c){cycles = c;}
	int getCycles(){return cycles;}
	int getContextSwitches(){return contextSwitches;}
	int getPreemptions(){return preemptions;}
};

class CPU{
	int cpuID;
	int ncores;	//Number of processing cores
	int t_cs;	//Time required for context switch
	int processTime;	//Time spent on current process
	int t_slice;
	int cycle;	//Number of cycles the processor has completed
	string scheduler;

	list<Process*> ioQueue;
	list<Process*> readyQueue;

	vector<Process> processes;
	vector<Core> cores;

public:
	CPU(int cID, int numCores, int timeCtx, int timeSlice, string sch);
	~CPU();
	void loadProcesses(char* filename);
	void printProcessStatistics();
	void printMsg(string msg);
	bool executeCycle();
	void executeIO();
	void executeFCFS();
	void executeSJF();
	void executeRR();
	int getCycles();
	void checkArrivals();
	void sortBurstTime(list<Process*>* queue);
	static bool compareBurstTime(Process* first, Process* second);
	void printStatistics(ofstream* out);
	void addWait();
};
#endif