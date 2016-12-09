#ifndef __PAGEREFERENCE_H_INCLUDED__
#define __PAGEREFERENCE_H_INCLUDED__

#include <map>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

//Page class is used by the frame just to store some basic information about the virtual memory frame
class page{
public:
	page(int pageNumber);
	int getPageNumber(){return pageNumber;}
	int getNumAccesses(){return numAccesses;}
	int getWaitTime(){return waitTime;}

	void cycle(){++waitTime;}
	void hitPage(){++numAccesses;}
	void setPageNumber(int n){pageNumber = n;}
private:
	int pageNumber;
	int numAccesses;
	int waitTime;
};

//This is the main virtual memory object:
class virtualMemoryDevice{
public:
	virtualMemoryDevice(string algorithm, char* filename);
	virtualMemoryDevice(string algorithm, char* filename, int frameSize);
	void printMsg(int ref, int vic);
	vector<page>::iterator findVictim();
	void readInputFile(char* filename);
	bool checkInsertNextReference();
	bool cycleMemDevice();


private:
	int frameSize;
	int currentCycle;
	int numPageFaults;
	string algorithm;
	vector<page> frame;
	list<int> pageReferences;
	page* emptyPage;

};

#endif