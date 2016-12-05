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
memDevice::memDevice(int s, int mmt, string style, string algo, int fpl){
	this->size = s;
	this->memMoveTime = mmt;
	this->algorithm = algo;
	this->currentCycle = 0;
	this->fpl = fpl;
	this->lastPlaced.clear();
	this->isFinished = false;
	this->style = style;
	this->freeMemProcess = new process(". -1 0/0");
	updateFreeSpace();
	stringstream msg;
	msg << "Simulator started (" << style << " -- " << algo << ")\n";
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


int memDevice::addTableEntry(process* uP, int start, int size, int d){
	dataEntry temp(uP->getProcessID(), start, size, d);
	if(uP->getProcessID() == '#'){
		this->freeSpace.push_back(temp);
	}
	else{
		map<process*, list<dataEntry> >::iterator i = pageTable.find(uP);
		this->data.push_back(temp);
		pageTable[uP].push_back(temp);
		this->lastPlaced.clear();
		this->lastPlaced.push_back(temp);
	}
	//cout << "ID: " << temp.getUserProcess() << " START: " << temp.getStart() << " END: " << temp.getEnd() << endl;
	return 0;
}


bool sortByLocation(dataEntry a, dataEntry b){
	return a.getEnd() < b.getStart();
}


bool sortByLocationPointer(dataEntry* a, dataEntry* b){
	return a->getEnd() < b->getStart();
}


int memDevice::defrag(){
	int defragTime = 0;
	//Sort data vector by start position in memory
	list<dataEntry*> sortList;
	for(map<process*, list<dataEntry> >::iterator i = pageTable.begin(); i != pageTable.end(); ++i){
		if(i->first != freeMemProcess){
			for(list<dataEntry>::iterator e = i->second.begin(); e != i->second.end(); ++e){
				sortList.push_back(&(*e));
			}
		}
	}
	sortList.sort(sortByLocationPointer);

	//For each element, shift backwards as far as possible:
	for(list<dataEntry*>::iterator d = sortList.begin(); d != sortList.end(); ++d){
		if(d == sortList.begin()){
			defragTime += (*d)->getEntrySize() * this->memMoveTime;//(d->getStart()) * this->memMoveTime;
			(*d)->shiftToLocation(0);
		}
		else{
			list<dataEntry*>::iterator prev = d;
			--prev;
			defragTime += (*d)->getEntrySize() * this->memMoveTime;
			(*d)->shiftToLocation((*prev)->getEnd() + 1);
		}
	}

	for(map<process*, list<dataEntry> >::iterator i = pageTable.begin(); i != pageTable.end(); ++i){
		if(i->first != freeMemProcess){
			i->first->delayNextUsageTime(defragTime);
			i->first->delayArrivalTimeRunning(defragTime);
		}
	}
	for(list<process*>::iterator i = this->waitingProcesses.begin(); i != this->waitingProcesses.end(); ++i){
		(*i)->delayArrivalTime(defragTime);
	}
	updateFreeSpace();
	return defragTime;
}





//Prints the memory based on the number of frames per line:
void memDevice::printMem(){
	cout << string(fpl, '=') << endl;
	for(int j = 0; j < (size/fpl); ++j){
		for(int i = 0; i < fpl; ++i){
			for(map<process*, list<dataEntry> >::iterator d = this->pageTable.begin(); d != this->pageTable.end(); ++d){
				for(list<dataEntry>::iterator e = d->second.begin(); e != d->second.end(); ++e){
					if(e->getStart() <= ((j*fpl) + i) && e->getEnd() >= ((j*fpl) + i)){
						cout << e->getUserProcess();
					}
				}
			}
		}
		cout << endl;
	}
	cout << string(fpl, '=') << endl;
}

void memDevice::printFreeSpace(){
	cout << string(fpl, '=') << endl;
	for(int j = 0; j < (size/fpl); ++j){
		for(int i = 0; i < fpl; ++i){
			bool printed = false;
			for(list<dataEntry>::iterator e = pageTable[freeMemProcess].begin(); e != pageTable[freeMemProcess].end(); ++e){
				if(e->getStart() <= ((j*fpl) + i) && e->getEnd() >= ((j*fpl) + i)){
					printed = true;
					cout << e->getUserProcess();
				}
			}
			if(!printed){
				cout << '#';
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

int memDevice::totalSize(process* uP){
	int size = 0;
	if(pageTable[uP].empty()){
		return 0;
	}
	else{
		for(list<dataEntry>::iterator i = pageTable[uP].begin(); i != pageTable[uP].end(); ++i){
			size += i->getEntrySize();
		}
	}
	return size;
}



int memDevice::insertMemoryContiguous(process* uP){
	bool placed = false;
	if(this->getAlgorithm() == "First-Fit"){
		for(list<dataEntry>::iterator i = pageTable[freeMemProcess].begin(); i != pageTable[freeMemProcess].end(); ++i){
			if(uP->getMemSize() < i->getEntrySize()){
				addTableEntry(uP, i->getStart(), uP->getMemSize(), uP->getNextUsageTime());
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
		pageTable[freeMemProcess].sort(sortByLocation);
		list<dataEntry>::iterator preSplit = pageTable[freeMemProcess].end();
		for(list<dataEntry>::iterator i = pageTable[freeMemProcess].begin(); i != pageTable[freeMemProcess].end(); ++i){
			//If we are before the split, we just store the empty slot we find first:
			if(i->getStart() - freeSpaceScanStart < 0){ 
				if(uP->getMemSize() <= i->getEntrySize() && preSplit == pageTable[freeMemProcess].end()){
					preSplit = i;
				}
			}
			else{ //If we are after the split, we use the find first algorithm:
				if(uP->getMemSize() < i->getEntrySize()){
					addTableEntry(uP, i->getStart(), uP->getMemSize(), uP->getNextUsageTime());
					updateFreeSpace();
					placed = true;
					break;
				}
			}
		}
		if(placed == false && preSplit != pageTable[freeMemProcess].end()){
			addTableEntry(uP, preSplit->getStart(), uP->getMemSize(), uP->getNextUsageTime());
			updateFreeSpace();
			placed = true;
		}
	}

	else if(this->getAlgorithm() == "Best-Fit"){
		int sizeMeter = this->getSize() + 1;
		list<dataEntry>::iterator t = pageTable[freeMemProcess].end();
		for(list<dataEntry>::iterator i = pageTable[freeMemProcess].begin(); i != pageTable[freeMemProcess].end(); ++i){
			if(i->getEntrySize() < sizeMeter && uP->getMemSize() <= i->getEntrySize()){
				t = i;
				sizeMeter = t->getEntrySize();
			}
		}
		if(t != pageTable[freeMemProcess].end()){
			addTableEntry(uP, t->getStart(), uP->getMemSize(), uP->getNextUsageTime());
			//cout << t->getStart() <<"\n";
			updateFreeSpace();
			//printFreeSpace(32);
			placed = true;
		}
	}

	else if(this->getAlgorithm() == "Worst-Fit"){
		int sizeMeter = 0;
		list<dataEntry>::iterator t = pageTable[freeMemProcess].end();
		for(list<dataEntry>::iterator i = pageTable[freeMemProcess].begin(); i != pageTable[freeMemProcess].end(); ++i){
			if(i->getEntrySize() > sizeMeter && uP->getMemSize() <= i->getEntrySize()){
				t = i;
				sizeMeter = t->getEntrySize();
			}
		}
		if(t != pageTable[freeMemProcess].end()){
			addTableEntry(uP, t->getStart(), uP->getMemSize(), uP->getNextUsageTime());
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
		printMem();
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

int memDevice::insertMemoryNonContiguous(process* uP){
	bool placed = false;
	//we do a simple first-fit algorithm:
	int memRemaining = uP->getMemSize();
	pageTable[freeMemProcess].sort(sortByLocation);
	for(list<dataEntry>::iterator i = pageTable[freeMemProcess].begin(); i != pageTable[freeMemProcess].end(); ++i){
		//If the slot is big enough to fit the object, awesome:
		if(memRemaining <= i->getEntrySize()){
			addTableEntry(uP, i->getStart(), memRemaining, uP->getNextUsageTime());
			placed = true;
			break;
		}
		if(i->getEntrySize() < memRemaining){
			addTableEntry(uP, i->getStart(), i->getEntrySize(), uP->getNextUsageTime());
			memRemaining -= i->getEntrySize();
		}					
	
	}
	updateFreeSpace();

	if(placed){
		stringstream msg;
		msg << "Placed process " << uP->getProcessID() << ":\n";
		printMsg(msg.str());
		printMem();
		return 0;
	}
	return -1;
}

int memDevice::insertMemory(process* uP){
	if(totalSize(freeMemProcess) < uP->getMemSize()){
		return -1;
	}
	if(this->style == "Contiguous"){
		return insertMemoryContiguous(uP);
	}
	else if(this->style == "Non-Contiguous"){
		return insertMemoryNonContiguous(uP);
		printMem();
	}
	else{
		return -1;
	}
}
 
void memDevice::printProcesses(){
	for(list<process*>::iterator i = waitingProcesses.begin(); i != waitingProcesses.end(); ++i){
		(*i)->printStatistics();
	}
}

void memDevice::updateFreeSpace(){
	pageTable[freeMemProcess].clear();
	dataEntry memInit(freeMemProcess->getProcessID(), 0, size, -1);
	pageTable[freeMemProcess].push_back(memInit); 
	for(map<process*, list<dataEntry> >::iterator p = pageTable.begin(); p != pageTable.end(); ++p){
		if(p->first != freeMemProcess){
			for(list<dataEntry>::iterator d = p->second.begin(); d != p->second.end(); ++d){
				//Here we resize the memory blocks around the process
				for(list<dataEntry>::iterator f = pageTable[freeMemProcess].begin();
					 						f != pageTable[freeMemProcess].end(); ++f){
					//Iterate through each member of the free space list
					if(f->getStart() <= d->getStart() && d->getEnd() <= f->getEnd()){
						if(f->getEnd()-d->getEnd() > 0){
							dataEntry temp(freeMemProcess->getProcessID(), d->getEnd() + 1, f->getEnd()-d->getEnd(), -1);
							pageTable[freeMemProcess].push_back(temp);
						}
						if(d->getStart()-f->getStart() > 0){
							f->setSize(d->getStart()-f->getStart());
						}
						else{
							f = pageTable[freeMemProcess].erase(f);
							--f;
						}

					}
				}
			}
		}
		else{
			//do nothing
		}
	}
}


/*
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
}*/

int memDevice::removeMemory(process* uP){
	pageTable.erase(uP);
	updateFreeSpace();
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
	updateFreeSpace();
}

void memDevice::checkFinished(){
	if(pageTable.size() != 1){
		this->isFinished = false;
		return;
	}
	for(list<process*>::iterator i = waitingProcesses.begin(); i != waitingProcesses.end(); ++i){
		if(!(*i)->checkEmpty()){
			//cout << (*i)->getProcessID() << ": " << (*i)->getNextArrivalTime() << endl;
			this->isFinished = false;
			return;
		}
	}
	stringstream msg;
	msg << "Simulator ended (" << this->style << " -- " << this->algorithm << ")\n";
	printMsg(msg.str());
	this->isFinished = true;
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
			printMem();
		}
		else{
			++i;
		}
	}
	updateFreeSpace();
}

void memDevice::cycleMemDevice(){
	checkWaiting();
	checkRunning();
	checkFinished();
	++currentCycle;

}