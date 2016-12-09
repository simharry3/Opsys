////////////////////////////////////////////////////////////
//Computer Operating Systems Project 2: Memory Management
//
//Written By: Clayton Rayment
//RCSID: raymec
//RIN: 661133772
///////////////////////////////////////////////////////////

#include "pageReference.h"


page::page(int pageNumber){
	this->pageNumber = pageNumber;
	numAccesses = 0;
	waitTime = 0;
}

//////////////////////////////////////////////////////////////////////////////

//Constructor for virtualMemoryDevice without given frame size:
virtualMemoryDevice::virtualMemoryDevice(string algorithm, char* filename){
	readInputFile(filename);
	emptyPage = new page(-1);
	frameSize = 3;
	for(int i = 0; i < frameSize; ++i){
		frame.push_back(*emptyPage);
	}
	this->algorithm = algorithm;
	currentCycle = 0;
	numPageFaults = 0;
	cout << "Simulating " << algorithm << " with fixed frame size of " << frameSize << endl;
}

//Constructor for virtualMemoryDevice with given frame size:
virtualMemoryDevice::virtualMemoryDevice(string algorithm, char* filename, int frameSize){
	readInputFile(filename);
	emptyPage = new page(-1);
	for(int i = 0; i < frameSize; ++i){
		frame.push_back(*emptyPage);
	}
	this->algorithm = algorithm;
	this->frameSize = frameSize;
	currentCycle = 0;
	numPageFaults = 0;
	cout << "Simulating " << algorithm << " with fixed frame size of " << frameSize;
}

//This function finds the next victim given the current running algorithm:
vector<page>::iterator virtualMemoryDevice::findVictim(){
	//Check to see if there are any empty slots:

	for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
		if(i->getPageNumber() == emptyPage->getPageNumber()){
			return i;
		}
	}

	vector<page>::iterator out = frame.end();
	
	//Algorithm for optimal replacement:
	if(algorithm == "OPT"){
		int maxAccess = 0;
		vector<page>::iterator minPage = frame.begin();
		for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
			int nextAccess = 0;
			for(list<int>::iterator j = pageReferences.begin(); j != pageReferences.end(); ++j){
				++nextAccess;
				if(i->getPageNumber() < minPage->getPageNumber()){
					minPage = i;
				}
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
			if(out == frame.end()){
				out = minPage;
			}
		}
	}

	//Algorithm for least-recently used replacement:
	else if(algorithm == "LRU"){
		out = frame.begin();
		int longestWait = 0;
		for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
			if(i->getWaitTime() > longestWait){
				longestWait = i->getWaitTime();
				out = i;
			}
			else if(i->getWaitTime() == longestWait && i->getPageNumber() < out->getPageNumber()){
				out = i;
			}
		}
	}

	//Algorithm for least-frequently used replacement:
	else if(algorithm == "LFU"){
		out = frame.begin();
		for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
			if(i->getNumAccesses() < out->getNumAccesses()){
				out = i;
			}
			else if(i->getNumAccesses() == out->getNumAccesses() && i->getPageNumber() < out->getPageNumber()){
				out = i;
			}
		}
	}

	return out;
}


//Prints a nice message to stdout since I'm lazy:
void virtualMemoryDevice::printMsg(int ref, int vic){
	cout << "referencing page " << ref << " [mem:";
	for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
		if(i->getPageNumber() != emptyPage->getPageNumber()){
			cout << " " << i->getPageNumber();
		}
		else{
			cout << " .";
		}
	}
	cout << "] PAGE FAULT (";
	if(vic != emptyPage->getPageNumber()){
		cout << "victim page " << vic;
	}
	else{
		cout << "no victim page";
	}
	cout << ")\n";

}

//Checks and inserts the next reference from the reference string
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
	int victimPageNumber = victim->getPageNumber();
	page temp(pageNumber);
	*victim = temp;
	printMsg(pageNumber, victimPageNumber);
	++numPageFaults;
	return false;
}

//Reads the input file and stores everything in the pageReferences list for ease of use:
void virtualMemoryDevice::readInputFile(char* filename){
	ifstream data;
	data.open(filename);
	char c;
	int i;
	while(data.get(c)){
		if(c != ' '){
			i = c - '0';
			pageReferences.push_back(c - '0');
		}
	}
	data.close();
}

//Main functional block of the virtual memory device, each "cycle" emulates a clock cycle:
bool virtualMemoryDevice::cycleMemDevice(){
	checkInsertNextReference();
	++currentCycle;
	for(vector<page>::iterator i = frame.begin(); i != frame.end(); ++i){
		i->cycle();
	}
	if(pageReferences.empty()){
		cout << "End of " << algorithm << " simulation (" << numPageFaults << " page faults)\n\n";

		frame.clear();
		delete emptyPage;
		return true;
	}
	return false;
}