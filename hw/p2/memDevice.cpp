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
memDevice::memDevice(int s){
	this->size = s;
}




//Adds a memory item to the data array, and initializes it:
int memDevice::addEntry(char uP, int start, int size, int d){
	dataEntry temp(uP, start, size, d);
	this->data.push_back(temp);
	return 0;
}


bool sortByLocation(dataEntry a, dataEntry b){
	return a.getEnd() < b.getStart();
}


int memDevice::defrag(){
	//Sort data vector by start position in memory
	sort(this->data.begin(), this->data.end(), sortByLocation);


	//For each element, shift backwards as far as possible:
	for(vector<dataEntry>::iterator d = this->data.begin(); d != this->data.end(); ++d){
		if(d == this->data.begin()){
			d->shiftToLocation(0);
		}
		else{
			vector<dataEntry>::iterator prev = d-1;
			d->shiftToLocation(prev->getEnd() + 1);
		}
	}

	return 0;
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