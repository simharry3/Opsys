#include "memDevice.h"

using namespace std;
//DATA ENTRY:
dataEntry::dataEntry(char uP, int s, int size, int d){
	setStart(s);
	setSize(size);
	setUserProcess(uP);
	setDuration(d);
	this->currentCycle = 0;
}


void dataEntry::shiftToLocation(int loc){ 
	setStart(loc);
}
//////////////////////////////////////////////////////////////////////////////////////////////////

//MEM DEVICE:
memDevice::memDevice(int s, int mmt, string algo, int fpl){
	this->size = s;
	this->memMoveTime = mmt;
	this->algorithm = algo;
	this->currentCycle = 0;
	this->fpl = fpl;
	this->lastPlaced.clear();
	this->isFinished = false;
	stringstream msg;
	msg << "Simulator started (" << algo << ")\n";
	printMsg(msg.str());
}




//Adds a memory item to the data array, and initializes it:
int memDevice::addEntry(char uP, int start, int size, int d){
	dataEntry temp(uP, start, size, d);
	if(uP == '#'){
		this->freeSpace.push_back(temp);
	}
	else{
		this->data.push_back(temp);
		this->lastPlaced.clear();
		this->lastPlaced.push_back(temp);
	}
	//cout << "ID: " << temp.getUserProcess() << " START: " << temp.getStart() << " END: " << temp.getEnd() << endl;
	return 0;
}


bool sortByLocation(dataEntry a, dataEntry b){
	return a.getEnd() < b.getStart();
}


int memDevice::defrag(){
	int defragTime = 0;
	//Sort data vector by start position in memory
	sort(this->data.begin(), this->data.end(), sortByLocation);

	//For each element, shift backwards as far as possible:
	for(vector<dataEntry>::iterator d = this->data.begin(); d != this->data.end(); ++d){
		if(d == this->data.begin()){
			defragTime += d->getEntrySize() * this->memMoveTime;//(d->getStart()) * this->memMoveTime;
			d->shiftToLocation(0);
		}
		else{
			vector<dataEntry>::iterator prev = d-1;
			defragTime += d->getEntrySize() * this->memMoveTime;
			d->shiftToLocation(prev->getEnd() + 1);
		}
	}
	updateFreeSpace();
	return defragTime;
}





//Prints the memory based on the number of frames per line:
void memDevice::printMem(int fpl){
	cout << string(fpl, '=') << endl;
	for(int j = 0; j < (size/fpl); ++j){
		for(int i = 0; i < fpl; ++i){
			bool inSlot = false;
			for(vector<dataEntry>::iterator d = this->data.begin(); d != this->data.end(); ++d){
				if(d->getStart() <= ((j*fpl) + i) && d->getEnd() >= ((j*fpl) + i)){
					cout << d->getUserProcess();
					inSlot = true;
				}
			}
			if(!inSlot){
				cout << '.';
			}
		}
		cout << endl;
	}
	cout << string(fpl, '=') << endl;
}

void memDevice::printFreeSpace(int fpl){
	cout << string(fpl, '=') << endl;
	for(int j = 0; j < (size/fpl); ++j){
		for(int i = 0; i < fpl; ++i){
			bool inSlot = false;
			for(vector<dataEntry>::iterator d = this->freeSpace.begin(); d != this->freeSpace.end(); ++d){
				if(d->getStart() <= ((j*fpl) + i) && d->getEnd() >= ((j*fpl) + i)){
					cout << d->getUserProcess();
					inSlot = true;
				}
			}
			if(!inSlot){
				cout << '.';
			}
		}
		cout << endl;
	}
	cout << string(fpl, '=') << endl;
}

void memDevice::loadProcesses(char* filename){
	ifstream data;
	data.open(filename);
	string line;
	while(getline (data, line)){
		if(line.find('#') == string::npos && line.length() != 0){
			process* temp = new process(line);
			waitingProcesses.push_back(temp);
			//addWaitingProcess(temp);
		}
	}
	data.close();
}

int memDevice::totalSize(vector<dataEntry>* dat){
	int size = 0;
	if(dat->empty()){
		return 0;
	}
	else{
		for(vector<dataEntry>::iterator i = dat->begin(); i != dat->end(); ++i){
			size += i->getEntrySize();
		}
	}
	return size;
}

int memDevice::insertMemory(process* uP){
	if(totalSize(&freeSpace) < uP->getMemSize()){
		return -1;
	}
	bool placed = false;
	if(this->getAlgorithm() == "First-Fit"){
		for(vector<dataEntry>::iterator i = freeSpace.begin(); i != freeSpace.end(); ++i){
			if(uP->getMemSize() < i->getEntrySize()){
				addEntry(uP->getProcessID(), i->getStart(), uP->getMemSize(), uP->getNextUsageTime());
				freeSpace.erase(i);
				updateFreeSpace();
				placed = true;
				break;
			}
		}	
	}

	else if(this->getAlgorithm() == "Next-Fit"){
		int freeSpaceScanStart = 0;
		if(lastPlaced.size() != 0){
			freeSpaceScanStart = lastPlaced.front().getEnd() + 1;
		}
		sort(freeSpace.begin(), freeSpace.end(), sortByLocation);
		vector<dataEntry>::iterator preSplit = freeSpace.end();
		for(vector<dataEntry>::iterator i = freeSpace.begin(); i != freeSpace.end(); ++i){
			//If we are before the split, we just store the empty slot we find first:
			if(i->getStart() - freeSpaceScanStart < 0){ 
				if(uP->getMemSize() <= i->getEntrySize() && preSplit == freeSpace.end()){
					preSplit = i;
				}
			}
			else{ //If we are after the split, we use the find first algorithm:
				if(uP->getMemSize() < i->getEntrySize()){
					addEntry(uP->getProcessID(), i->getStart(), uP->getMemSize(), uP->getNextUsageTime());
					freeSpace.erase(i);
					updateFreeSpace();
					placed = true;
					break;
				}
			}
		}
		if(placed == false && preSplit != freeSpace.end()){
			addEntry(uP->getProcessID(), preSplit->getStart(), uP->getMemSize(), uP->getNextUsageTime());
			freeSpace.erase(preSplit);
			updateFreeSpace();
			placed = true;
		}
	}

	else if(this->getAlgorithm() == "Best-Fit"){
		int sizeMeter = this->getSize() + 1;
		vector<dataEntry>::iterator t = freeSpace.end();
		for(vector<dataEntry>::iterator i = freeSpace.begin(); i != freeSpace.end(); ++i){
			if(i->getEntrySize() < sizeMeter && uP->getMemSize() <= i->getEntrySize()){
				t = i;
				sizeMeter = t->getEntrySize();
			}
		}
		if(t != freeSpace.end()){
			addEntry(uP->getProcessID(), t->getStart(), uP->getMemSize(), uP->getNextUsageTime());
			//cout << t->getStart() <<"\n";
			updateFreeSpace();
			//printFreeSpace(32);
			placed = true;
		}
	}

	else if(this->getAlgorithm() == "Worst-Fit"){
		int sizeMeter = 0;
		vector<dataEntry>::iterator t = freeSpace.end();
		for(vector<dataEntry>::iterator i = freeSpace.begin(); i != freeSpace.end(); ++i){
			if(i->getEntrySize() > sizeMeter && uP->getMemSize() <= i->getEntrySize()){
				t = i;
				sizeMeter = t->getEntrySize();
			}
		}
		if(t != freeSpace.end()){
			addEntry(uP->getProcessID(), t->getStart(), uP->getMemSize(), uP->getNextUsageTime());
			//cout << t->getStart() <<"\n";
			updateFreeSpace();
			//printFreeSpace(32);
			placed = true;
		}
	}

	if(placed){
		stringstream msg;
		msg << "Placed process " << uP->getProcessID() << ":\n";
		printMsg(msg.str());
		printMem(this->fpl);
		return 0;
	}
	else{
		stringstream msg;
		msg << "Could not place process " << uP->getProcessID() << " -- starting defragmentation\n";
		printMsg(msg.str());
		this->currentCycle += defrag();
		return insertMemory(uP);
	}
}

void memDevice::printProcesses(){
	for(list<process*>::iterator i = waitingProcesses.begin(); i != waitingProcesses.end(); ++i){
		(*i)->printStatistics();
	}
}


void memDevice::updateFreeSpace(){
	freeSpace.clear();
	sort(this->data.begin(), this->data.end(), sortByLocation);
	char nP = '#';
	if(data.size() == 0){
		dataEntry temp(nP, 0, this->getSize(), -1);
		this->freeSpace.push_back(temp);
	}
	else{
		vector<dataEntry>::iterator i = data.begin();
		for(; i != data.end(); ++i){
			if(i == data.begin()){
				if(i->getStart() != 0){
					dataEntry temp(nP, 0, i->getStart(), -1);
					// cout << "SIZE: " << temp.getEntrySize() << " START: "
					// 	 << temp.getStart() << " END: " << temp.getEnd() << endl;
					addEntry(nP, 0, i->getStart(), -1);
				}
				
			}
			else{
				vector<dataEntry>::iterator prev = i-1;
				if(i->getStart() - prev->getEnd() > 1){
					dataEntry temp(nP, prev->getEnd() + 1, i->getStart() - prev->getEnd() - 1, -1);
					// cout << "SIZE: " << temp.getEntrySize() << " START: "
					// 	 << temp.getStart() << " END: " << temp.getEnd() << endl;
					addEntry(nP, prev->getEnd() + 1, i->getStart() - prev->getEnd() - 1, -1);
				}
			}
		}
		//Check final value:
		--i;
		if(this->getSize() - i->getEnd() > 1){
			dataEntry temp(nP, i->getEnd() + 1, this->getSize() - i->getEnd() - 1, -1);
			// cout << "SIZE: " << temp.getEntrySize() << " START: "
			// 			 << temp.getStart() << " END: " << temp.getEnd() << endl;
			addEntry(nP, i->getEnd() + 1, this->getSize() - i->getEnd() - 1, -1);
		}
	}
}

int memDevice::removeMemory(process* uP){
	vector<dataEntry>::iterator i = data.begin();
	while(i != data.end()){
		if(i->getUserProcess() == uP->getProcessID()){
			i = data.erase(i);
		}
		else{
			++i;
		}
	}
	return 0;
}

void memDevice::checkWaiting(){
	list<process*>::iterator i = waitingProcesses.begin();
	while(i != waitingProcesses.end()){
		if((*i)->getNextArrivalTime() == getDeviceCycle()){
			stringstream msg;
			msg << "Process " << (*i)->getProcessID() << " arrived (requires " 
											<< (*i)->getMemSize() << " frames)\n";
			printMsg(msg.str());
			if(insertMemory(*i) == 0){
				runningProcesses.push_back(*i);
				i = waitingProcesses.erase(i);
			}
			else{
				stringstream msg;
				msg << "Cannot place process " << (*i)->getProcessID() << " -- skipped!\n";
				printMsg(msg.str());
				(*i)->completeCycle();
				++i;
			}
		}
		else{
			++i;
		}
	}
}
bool memDevice::checkFinished(){
	for(list<process*>::iterator i = waitingProcesses.begin()){
		if(!(*i)->checkEmpty()){
			return false;
		}
	}
	return true;
}
void memDevice::printMsg(string msg){
	cout << "time " << currentCycle << "ms: " << msg;
}

void memDevice::checkRunning(){
	list<process*>::iterator i = runningProcesses.begin();
	while(i != runningProcesses.end()){
		if((*i)->getNextUsageTime() == getDeviceCycle() - (*i)->getNextArrivalTime()){
			(*i)->completeCycle();
			waitingProcesses.push_back(*i);
			stringstream msg;
			msg << "Process " << (*i)->getProcessID() << " removed:\n";
			printMsg(msg.str());
			removeMemory(*i);
			i = runningProcesses.erase(i);
			printMem(this->fpl);
		}
		else{
			++i;
		}
	}
}

void memDevice::cycleMemDevice(){
	checkWaiting();
	updateFreeSpace();
	checkRunning();
	updateFreeSpace();

	++currentCycle;

}