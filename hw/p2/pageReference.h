#include <map>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

class page{
public:
	page(int pageNumber);
	int getPageNumber(){return pageNumber;}
	int getNumAccesses(){return numAccesses;}

	void hitPage(){++numAccesses;}
	void setPageNumber(int n){pageNumber = n;}
private:
	int pageNumber;
	int numAccesses;
	int waitTime;
};

class virtualMemoryDevice{
public:
	virtualMemoryDevice(string algorithm, char* filename);
	virtualMemoryDevice(string algorithm, int frameSize, char* filename);
	int findVictim();
	void readInputFile(char* filename);
	bool checkInsertNextReference();
	bool cycleMemDevice();


private:
	int frameSize;
	int currentCycle;
	string algorithm;
	vector<page> frame;
	list<int> pageReferences;
	page emptyPage;

};