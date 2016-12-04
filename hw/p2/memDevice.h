#ifndef __MEMDEVICE_H_INCLUDED__
#define __MEMDEVICE_H_INCLUDED__

#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include "process.h"

using namespace std;

class dataEntry{
public:
	dataEntry(char uP, int mS, int mSize, int d);
	//SETTERS:
	void setUserProcess(char uP){this->userProcess = uP;}
	void setStart(int s){this->memStart = s;}
	void setSize(int size){this->memSize = size;}
	void setDuration(int d){this->duration = d;}
	void cycle(){++currentCycle;}
	//GETTERS:
	char getUserProcess(){return this->userProcess;}
	int getStart(){return this->memStart;}
	int getEnd(){return this->memStart + this->memSize - 1;}
	int getDuration(){return this->duration;}
	int getCycle(){return this->currentCycle;}
	int getEntrySize(){return this->memSize;}
	//HELPERS:
	void shiftToLocation(int loc);

private:
	char userProcess;
	int memStart;
	int memSize;
	int currentCycle;
	int duration;
};

class memDevice{

public:
	memDevice(int s, int mmt, string algorithm, int fpl);
	//SETTERS:
	void setSize(int s){this->size = s;}
	void addReadyProcess(process* p){readyProcesses.push_back(p);}
	void addWaitingProcess(process* p){waitingProcesses.push_back(p);}
	
	//GETTERS:
	int getSize(){return this->size;}
	int getDeviceCycle(){return this->currentCycle;}
	string getAlgorithm(){return this->algorithm;}
	//HELPERS:
	int addEntry(char uP, int start, int size, int d);
	int insertMemory(process* uP);
	int removeMemory(process* uP);
	int defrag();
	void printMem(int fpl);
	void loadProcesses(char* filename);
	void printProcesses();
	void updateFreeSpace();
	void printFreeSpace(int fpl);
	void printMsg(string msg);
	int totalSize(vector<dataEntry>* dat);

	//Possibly move to PROCESSOR class:
	void checkWaiting();
	void checkReady();
	void checkRunning();
	void cycleMemDevice();
	bool checkFinished();

private:
	int size;
	int memMoveTime;
	int currentCycle;
	int fpl;
	list<dataEntry> lastPlaced;
	string algorithm;

	vector<dataEntry> freeSpace;
	vector<dataEntry> data;
	list<process*> runningProcesses;
	list<process*> readyProcesses;
	list<process*> waitingProcesses;
};

#endif