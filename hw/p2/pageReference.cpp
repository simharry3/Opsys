#include "pageReference.h"


page::page(int pageNumber){
	this->pageNumber = pageNumber;
	numAccesses = 0;
	waitTime = 0;
}



virtualMemoryDevice::virtualMemoryDevice(string algorithm, char* filename){
	readInputFile(filename);
	this->algorithm = algorithm;
	frameSize = 3;
	currentCycle = 0;

}

virtualMemoryDevice::virtualMemoryDevice(string algorithm, int frameSize, char* filename){
	readInputFile(filename);
	this->algorithm = algorithm;
	this->frameSize = frameSize;
	currentCycle = 0;
}

int virtualMemoryDevice::findVictim(){
	if(algorithm == "OPT"){
		int nextAccessTime = 0;
		for(list<int>::iterator i = pageReferences.begin(); i != pageReferences.end(); ++i){

		}
	}
	else if(algorithm == "LRU"){
		int longestWait = 0;

	}
	else if(algorithm == "LFU"){

	}
}

bool virtualMemoryDevice::checkInsertNextReference(){
	int pageNumber = pageReferences.front();
	pageReferences.pop_front();
	for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
		if(i->getPageNumber() == pageNumber){
			i->hitPage();
			return true;
		}
	}
	int victim = findVictim();
	return false;
}

void virtualMemoryDevice::readInputFile(char* filename){
	ifstream data;
	data.open(filename);
	char c;
	while(data.get(c)){
		pageReferences.push_back(atoi(&c));
	}
	data.close();
}


bool virtualMemoryDevice::cycleMemDevice(){
	checkInsertNextReference();
	++currentCycle;
	if(pageReferences.empty()){
		return true;
	}
	return false;
}