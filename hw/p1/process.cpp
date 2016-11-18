#include "process.h"

using namespace std;



Process::Process(string data){
	string delim = "|";
	size_t pos = 0;
	list<string> parsedData;
	while((pos = data.find(delim))!= string::npos){
		parsedData.push_back(data.substr(0, pos));
		data.erase(0, pos + delim.length());
	}
	parsedData.push_back(data);
	id = parsedData.front();
	parsedData.pop_front();
	istringstream(parsedData.front()) >> initialArrivalTime;
	parsedData.pop_front();
	istringstream(parsedData.front()) >> cpuBurstTime;
	parsedData.pop_front();
	istringstream(parsedData.front()) >> numBursts;
	parsedData.pop_front();
	istringstream(parsedData.front()) >> ioTime;
	parsedData.pop_front();
	runtime = 0;
	progress = 0;
	burstProgress = 0;
	ioProgress = 0;

	waitArrival = 0;
	burstArrival = 0;
	burstTime = 0;
	status = "NOT ARRIVED";
}

void Process::startWait(int t){
	//cout << ">>>>>>>>PROCESS " + id + " STARTED WAIT COMPUTATION<<<<<<<<<<<\n";
	waitArrival = t;
}

void Process::endWait(int t){
	int temp = t - waitArrival;
	//cout << ">>>>>>>>PROCESS " + id + " ENDED WAIT COMPUTATION: " + to_string(temp) + "ms<<<<<<<<<\n";
	waitV.push_back(temp);
	waitArrival = 0;
}

int Process::getWaitTime(){
	int temp = 0;
	for(vector<int>::iterator i = waitV.begin(); i != waitV.end(); ++i){
		temp += *i;
	}
	return temp;
}

void Process::start(int t){
	burstArrival = t;
}

void Process::end(int t){
	int temp = t - burstArrival;
	turnaroundV.push_back(temp);
	burstArrival = 0;
	//cout << ">>>>>>>>PROCESS " + id + " ENDED TURNAROUND COMPUTATION: " + to_string(temp) + "ms\n";
}

int Process::getTurnaroundTime(){
	int temp = 0;
	for(vector<int>::iterator i = turnaroundV.begin(); i != turnaroundV.end(); ++i){
		temp += *i;
	}
	return temp;
}




int Process::getCPUBurstTime() const{
	const int temp = cpuBurstTime;
	return temp;
}

int Process::getIOTime(){
	return ioTime;
}

string Process::checkStatus(){
	return status;
}
string Process::getID(){
	return id;
}
void Process::setStatus(string stat){
	status = stat;
}

int Process::getArrivalTime(){
	return initialArrivalTime;
}

bool Process::runAndCheckIO(){
	++ioProgress;
	if(ioProgress > ioTime){
		ioProgress = 0;
		status = "READY";
		return true;
	}
	return false;
}

bool Process::runAndCheckComplete(){
	// status = "RUNNING";
	if(status != "BLOCKED"){
		++progress;
		if(progress == cpuBurstTime){
			progress = 0;
			++burstProgress;
			if(burstProgress == numBursts){
				status = "COMPLETE";
			}
			else{
				status = "BLOCKED";
			}
			return true;
		}
	}
	return false;
}

void Process::printStatistics(){
	cout << "=====================================\n";
	cout << "Process ID: " << id << '\n';
	cout << "Initial Arrival Time: " << initialArrivalTime << "ms\n";
	cout << "Total running time: " << runtime << "ms\n";
	cout << "CPU Burst Time: " << cpuBurstTime << "ms\n";
	cout << "Number of Bursts: " << numBursts << '\n';
	cout << "Burst Progress: " << burstProgress << '\n';
	cout << "I/O Time: " << ioTime << "ms\n";
	cout << "I/O Progress: " << ioProgress << "ms\n";
	cout << "Burst Progress: " << progress << "ms\n";
	cout << "Status: " << status << "\n";
	cout << "=====================================\n";
}