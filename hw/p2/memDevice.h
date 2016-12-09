////////////////////////////////////////////////////////////
//Computer Operating Systems Project 2: Memory Management
//
//Written By: Clayton Rayment
//RCSID: raymec
//RIN: 661133772
///////////////////////////////////////////////////////////

#ifndef __MEMDEVICE_H_INCLUDED__
#define __MEMDEVICE_H_INCLUDED__

#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include "process.h"
#include <utility>
#include <map>

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
	memDevice(int s, int mmt, string style, string algorithm, int fpl);
	//SETTERS:
	void setSize(int s){this->size = s;}
	void addReadyProcess(process* p){readyProcesses.push_back(p);}
	void addWaitingProcess(process* p){waitingProcesses.push_back(p);}
	
	//GETTERS:
	int getSize(){return this->size;}
	int getDeviceCycle(){return this->currentCycle;}
	string getAlgorithm(){return this->algorithm;}
	bool getStatus(){return this->isFinished;}
	//HELPERS:
	int addEntry(char uP, int start, int size, int d);
	int addTableEntry(process* uP, int start, int size, int d);
	int insertMemory(process* uP);
	int insertMemoryContiguous(process* uP);
	int insertMemoryNonContiguous(process* uP);
	int removeMemory(process* uP);
	int defrag();
	void printMem();
	void loadProcesses(char* filename);
	void printProcesses();
	void updateFreeSpace();
	void printFreeSpace();
	void printMsg(string msg);
	int totalSize(process* uP);


	//Possibly move to PROCESSOR class:
	void checkWaiting();
	void checkReady();
	void checkRunning();
	void cycleMemDevice();
	void checkFinished();

private:
	int size;
	int memMoveTime;
	int currentCycle;
	int fpl;
	bool isFinished;
	list<dataEntry> lastPlaced;
	string algorithm;
	string style;



	vector<dataEntry> freeSpace;
	vector<dataEntry> data;
	map<process*, list<dataEntry> > pageTable;
	list<process*> runningProcesses;
	list<process*> readyProcesses;
	list<process*> waitingProcesses;
	process* freeMemProcess;
};

#endif