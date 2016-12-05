#include "process.h"

process::process(string initializer){
	string delim = " ";
	size_t pos = 0;
	list<string> parsedData;
	while((pos = initializer.find(delim))!= string::npos){
		parsedData.push_back(initializer.substr(0, pos));
		initializer.erase(0, pos + delim.length());
	}
	parsedData.push_back(initializer);

	this->processID = parsedData.front()[0];
	parsedData.pop_front();
	istringstream(parsedData.front()) >> this->memSize;
	parsedData.pop_front();
	
	for(list<string>::iterator i = parsedData.begin(); i != parsedData.end(); ++i){
		string delim2 = "/";
		pos = 0;
		list<string> data;
		int item;
		while((pos = i->find(delim2))!= string::npos){
			data.push_back(i->substr(0, pos));
			i->erase(0, pos + delim2.length());
		}
		data.push_back(*i);
		istringstream(data.front()) >> item;
		arrivalTimes.push_back(item);
		data.pop_front();
		istringstream(data.front()) >> item;
		usageTimes.push_back(item);
		data.pop_front();
		
	}
}

void process::completeCycle(){
	arrivalTimes.pop_front();
	usageTimes.pop_front();
}

void process::delayArrivalTime(int t){
	for(list<int>::iterator i = arrivalTimes.begin(); i != arrivalTimes.end(); ++i){
		*i += t;
	}
}

void process::delayArrivalTimeRunning(int t){
	if((arrivalTimes.size() > 1)){
		for(list<int>::iterator i = ++arrivalTimes.begin(); i != arrivalTimes.end(); ++i){
			*i += t;
		}
	}
}

void process::delayNextUsageTime(int t){
	int uT = usageTimes.front();
	usageTimes.pop_front();
	usageTimes.push_front(uT + t);
}


void process::printStatistics(){
	cout << "==========================\n";
	cout << "PROCESS ID: " << getProcessID() << endl;
	cout << "MEMORY SIZE: " << getMemSize() << endl;
	cout << endl;
	cout << "ARRIVAL/USAGE TIMES:\n";
	cout << "------------------------\n";
	list<int>::iterator arrival = this->arrivalTimes.begin();
	list<int>::iterator usage = this->usageTimes.begin();
	while(arrival != this->arrivalTimes.end() && this->arrivalTimes.size() != 0){
		cout << "> " << *arrival << "ms / " << *usage << "ms\n";
		++arrival;
		++usage;
	}
	cout << "==========================\n";
}