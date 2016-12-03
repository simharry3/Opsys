#ifndef __PROCESSOR_H_INCLUDED__
#define __PROCESSOR_H_INCLUDED__

#include <vector>
#include <iostream>
#include <algorithm>

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
	memDevice(int s);
	//SETTERS:
	void setSize(int s){this->size = s;}
	
	//GETTERS:
	int getSize(){return this->size;}

	//HELPERS:
	int addEntry(char uP, int start, int size, int d);
	int insertMemory(char fitAlgorithm);
	int defrag();
	void printMem(int fpl);

private:
	int size;
	vector<dataEntry> data;
};

#endif