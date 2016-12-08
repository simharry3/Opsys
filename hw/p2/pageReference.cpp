#include "pageReference.h"


page::page(int pageNumber){
	this->pageNumber = pageNumber;
	numAccesses = 0;
	waitTime = 0;
}



virtualMemoryDevice::virtualMemoryDevice(string algorithm, char* filename){
	readInputFile(filename);
	page* temp = new page(-1);
	emptyPage = temp;
	this->algorithm = algorithm;
	frameSize = 3;
	currentCycle = 0;

}

virtualMemoryDevice::virtualMemoryDevice(string algorithm, int frameSize, char* filename){
	readInputFile(filename);
	page* temp = new page(-1);
	emptyPage = temp;
	this->algorithm = algorithm;
	this->frameSize = frameSize;
	currentCycle = 0;
}

vector<page>::iterator virtualMemoryDevice::findVictim(){
	//Check to see if there are any empty slots:

	for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
		if(i->getPageNumber() == emptyPage->getPageNumber()){
			return i;
		}
	}

	vector<page>::iterator out;

	if(algorithm == "OPT"){
		int maxAccess = 0;
		for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
			int nextAccess = 0;
			for(list<int>::iterator j = pageReferences.begin(); j != pageReferences.end(); ++j){
				++nextAccess;
				if(i->getPageNumber() == *j){
					if(nextAccess > maxAccess){
						out = i;
						maxAccess = nextAccess;
						break;
					}
					else if(nextAccess == maxAccess && i->getPageNumber() < out->getPageNumber()){
						out = i;
						break;
					}
				}
			}
		}
	}

	else if(algorithm == "LRU"){
		int longestWait = 0;
		for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
			if(i->getWaitTime() > longestWait){
				longestWait = i->getWaitTime();
			}
		}
	}
	else if(algorithm == "LFU"){
		int lowestFrequency = 0;
		for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){

		}
	}

	return out;
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
	vector<page>::iterator victim = findVictim();
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
	for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
		i->cycle();
	}
	if(pageReferences.empty()){
		return true;
	}
	return false;
}