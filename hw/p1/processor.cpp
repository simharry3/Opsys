//Written by Clayton Rayment
//Defines Processor class and helper functions which will allow for better extendability to future projects
#include "processor.h"


using namespace std;


Core::Core(int cID, int tL, int tU, int tS){
	coreID = cID;
	activeProcess = NULL;
	status = "READY";
	runningTime = 0;
	timeSlice = tS;
	timeSliceTracker = 0;
	t_l = tL;
	t_u = tU;
	loadQueue = NULL;
	unloadQueue = NULL;
	preemptions = 0;
	contextSwitches = 0;
}

Core::~Core(){
	loadQueue = NULL;
	unloadQueue = NULL;
	activeProcess = NULL;
}

void Core::setActiveProcess(Process* proc){
	activeProcess = proc;
	if(proc != NULL){
		activeProcess->setStatus("RUNNING");
	}
}

vector<string> Core::getMessages(){
	vector<string> temp = messages;
	messages.erase(messages.begin(), messages.end());
	return temp;
}

string Core::getStatus(){
	return status;
}

Process* Core::getActiveProcess(){
	return activeProcess;
}

CPU::CPU(int cID, int numCores, int timeCtx, int timeSlice, string sch){
	cpuID = cID;
	ncores = numCores;
	t_cs = timeCtx;
	t_slice = timeSlice;
	int tU = t_cs/2;
	int tL = tU;
	scheduler = sch;
	processTime = 0;
	cycle = 0;
	for(int i = 0; i < numCores; ++i){
		Core temp (i, tU, tL, timeSlice);
		cores.push_back(temp);
	}
	string msg = "Simulator started for " + sch;
	printMsg(msg);
}

CPU::~CPU(){
	ioQueue.erase(ioQueue.begin(), ioQueue.end());
	readyQueue.erase(readyQueue.begin(), readyQueue.end());
}

bool CPU::compareBurstTime(Process* first, Process* second){
	return first->getCPUBurstTime() < second->getCPUBurstTime();
}

int CPU::getCycles(){
	return cycle;
}

void CPU::loadProcesses(char* filename){
	ifstream data;
	data.open(filename);
	string line;
	while(getline (data, line)){
		if(line.find('#') == string::npos && line.length() != 0){
			Process temp (line);
			if(temp.getIOTime() < t_cs/2){
				temp.setIO(0);
			}
			else{
				temp.setIO(temp.getIOTime()-t_cs/2);
			}
			processes.push_back(temp);
		}
	}
	data.close();
}

void CPU::printProcessStatistics(){
	for(vector<Process>::iterator i = processes.begin(); i != processes.end(); ++i){
		i->printStatistics();
	}
}

Process* Core::execute(bool processWaiting){
	Process* temp = NULL;
	if(status == "LOADING PROCESS"){
		if(runningTime == t_l){
			loadQueue = NULL;
			string msg = "Process " + activeProcess->getID() + " started using the CPU";
			messages.push_back(msg);
			activeProcess->setStatus("RUNNING");
			++contextSwitches;
			status = "RUNNING";
		}
	}

	else if(status == "RUNNING"){
		++timeSliceTracker;
		if(activeProcess->runAndCheckComplete()){
			if(activeProcess->checkStatus() == "COMPLETE"){
				string msg = "Process " + activeProcess->getID() + " terminated";
				messages.push_back(msg);
				activeProcess->end(cycles);
			}
			else{
				int burstRemaining = activeProcess->burstRemaining();
				stringstream ss;
				ss << burstRemaining;
				string br = ss.str();
				string msg = "Process " + activeProcess->getID() + " completed a CPU burst; " 
					+ br + " to go";
				messages.push_back(msg);
				activeProcess->end(cycles);
			}
			timeSliceTracker = 0;
			unloadProcess();
		}
		else if(timeSliceTracker == timeSlice){
			timeSliceTracker = 0;
			if(processWaiting){
				int timeRemaining = activeProcess->timeRemaining();
				stringstream ss;
				ss << timeRemaining;
				string tr = ss.str();
				string msg = "Time slice expired; process " + activeProcess->getID() + " preempted with "
					+ tr + "ms to go";
				messages.push_back(msg);
				activeProcess->setStatus("PREEMPTED");
				++preemptions;
				unloadProcess();
			}
			else{
				string msg = "Time slice expired; no preemption because ready queue is empty";
				messages.push_back(msg);
			}
		}
	}

    else if(status == "UNLOADING PROCESS"){
		if(runningTime == t_u - 1){
			cout << "UNLOADING\n";
			temp = activeProcess;
			activeProcess = NULL;
			status = "READY";
		}

	}
	++runningTime;
	return temp;
}

void Core::loadProcess(Process* proc){
	// cout << "LOADING START\n";
	runningTime = 0;
	loadQueue = proc;
	activeProcess = proc;
	activeProcess->endWait(cycles);
	status = "LOADING PROCESS";
}
void Core::unloadProcess(){
	// cout << "UNLOADING START\n";
	if(activeProcess->checkStatus() == "BLOCKED"){
		int blockedUntil = (getCycles() + activeProcess->getIOTime() + t_l);
		stringstream ss;
		ss << blockedUntil;
		string bt = ss.str();
		string msg = "Process " + activeProcess->getID() + " blocked on I/O until time "
			+ bt + "ms";
		messages.push_back(msg);
	}
	// else if(activeProcess->checkStatus() != "COMPLETE"){
	// 	activeProcess->end(cycles);
	// }

	runningTime = 0;
	unloadQueue = activeProcess;
	status = "UNLOADING PROCESS";
	// if(activeProcess->checkStatus() != "COMPLETE"){
	// 	activeProcess->setStatus("BLOCKED");
	// }
}

Process* Core::getLoadQueue(){
	return loadQueue;
}

void CPU::executeIO(){
	//cout << "CYCLE: " << cycle << " LIST SIZE: " << ioQueue.size() << "\n";
	if(!ioQueue.empty()){
		for(list<Process*>::iterator i = ioQueue.begin(); i != ioQueue.end(); ++i){
			if((*i)->runAndCheckIO()){
				list<Process*>::iterator j = i;
				if((*i)->checkStatus() != "COMPLETE"){
					readyQueue.push_back(*i);
					(*i)->start(getCycles());
					(*i)->startWait(getCycles());
					string msg = "Process " + (*i)->getID() + " completed I/O";
					printMsg(msg);
				}
				ioQueue.erase(j);
				--i;
			}
		}
	}
}

void CPU::checkArrivals(){
	for(vector<Process>::iterator i = processes.begin(); i != processes.end(); ++i){
		if(i->checkStatus() == "NOT ARRIVED" && cycle >= i->getArrivalTime()){
			readyQueue.push_back(&(*i));
			i->start(getCycles());
			i->startWait(getCycles());
			string temp = "Process " + i->getID() + " arrived";
			printMsg(temp);
			i->setStatus("READY");
		}
	}
}

void CPU::printMsg(string msg){
	cout << "time " << getCycles() << "ms: " << msg << " [Q";
	
	if(readyQueue.size() == 0){
		cout << " empty";
	}
	else{
		for(list<Process*>::iterator i = readyQueue.begin(); i != readyQueue.end(); ++i){
			cout << " " << (*i)->getID();
		}
	}
	
	cout << "]\n";
}

void CPU::printStatistics(ofstream* out){
	float totalBurst = 0;
	float totalWait = 0;
	float totalTurnaround = 0;
	int ctxSwitch = 0;
	int preempt = 0;
	int numBursts = 0;
	int n = 0;
	for(vector<Core>::iterator i = cores.begin(); i != cores.end(); ++i){
		for(vector<Process>::iterator j = processes.begin(); j != processes.end(); ++j){
			totalBurst += j->getBurstTime();
			totalWait += j->getWaitTime();
			totalTurnaround += j->getTurnaroundTime();
			numBursts += j->getNumBursts();
			++n;
		}
		ctxSwitch += i->getContextSwitches();
	    preempt += i->getPreemptions();
	}

	*out << "Algorithm " << scheduler << "\n";
	//*out << "NUMBER OF BURSTS: " + to_string(numBursts) + "\n";
	*out << fixed << setprecision(2);
	*out << "--average CPU burst time: " << totalBurst/numBursts << " ms\n";
	*out << "--average wait time: " << totalWait/numBursts << " ms\n";
	*out << "--average turnaround time: " << totalTurnaround/numBursts << " ms\n";
	*out << "--total number of context switches: " << ctxSwitch << "\n";
	*out << "--total number of preemptions: " << preempt << "\n";
}

void CPU::addWait(){
	for(list<Process*>::iterator i = readyQueue.begin(); i != readyQueue.end(); ++i){
		//(*i)->startWait();
	}
	for(vector<Core>::iterator i = cores.begin(); i != cores.end(); ++i){
		if(i->getLoadQueue() != NULL){
			//i->getLoadQueue()->startWait();
		}
	}
}

//Single CPU Tick Execution of FCFS Algorithm:
void CPU::executeFCFS(){
	checkArrivals();
	executeIO();

	for(vector<Core>::iterator i = cores.begin(); i != cores.end(); ++i){
		i->setCycles(getCycles());
		if(readyQueue.front() != NULL && i->getStatus() == "READY"){
			i->loadProcess(readyQueue.front());
			//i->getActiveProcess()->endWait(getCycles());
			readyQueue.pop_front();
		}
		Process* temp = i->execute(readyQueue.front() != NULL);
		vector<string> retMsg = i->getMessages();
		for(vector<string>::iterator j = retMsg.begin(); j != retMsg.end(); ++j){
			printMsg(*j);
		}
		if(temp != NULL){
			if(temp->checkStatus() != "COMPLETE"){
				ioQueue.push_back(temp);
			}
		}
	}
}


//Single CPU Tick Execution of SJF Algorithm:
void CPU::executeSJF(){
	checkArrivals();
	executeIO();

	for(vector<Core>::iterator i = cores.begin(); i != cores.end(); ++i){
		i->setCycles(getCycles());
		if(readyQueue.front() != NULL && i->getStatus() == "READY"){
			readyQueue.sort(compareBurstTime);
			i->loadProcess(readyQueue.front());
			readyQueue.pop_front();
		}
		Process* temp = i->execute(readyQueue.front() != NULL);

		vector<string> retMsg = i->getMessages();
		for(vector<string>::iterator j = retMsg.begin(); j != retMsg.end(); ++j){
			printMsg(*j);
		}
		if(temp != NULL){
			if(temp->checkStatus() != "COMPLETE"){
				ioQueue.push_back(temp);
			}
		}
	}
}

//Single CPU Tick Execution of RR Algorithm:
void CPU::executeRR(){
	checkArrivals();
	executeIO();

	for(vector<Core>::iterator i = cores.begin(); i != cores.end(); ++i){
		i->setCycles(getCycles());
		if(readyQueue.front() != NULL && i->getStatus() == "READY"){
			i->loadProcess(readyQueue.front());
			readyQueue.pop_front();
		}
		Process* temp = i->execute(readyQueue.front() != NULL);

		if(temp != NULL){
			if(temp->checkStatus() == "PREEMPTED"){
				readyQueue.push_back(temp);
				temp->setStatus("READY");
				temp->startWait(getCycles());
			}
			else if(temp->checkStatus() != "COMPLETE"){
				ioQueue.push_back(temp);
			}
		}
		vector<string> retMsg = i->getMessages();
		for(vector<string>::iterator j = retMsg.begin(); j != retMsg.end(); ++j){
			printMsg(*j);
		}
	}
}


bool CPU::executeCycle(){
	bool processComplete = true;
	bool coreComplete = true;
	if(scheduler == "FCFS"){
		executeFCFS();
		//printMsg(" ");
	}
	else if(scheduler == "SJF"){
		executeSJF();
	}
	else if(scheduler == "RR"){
		executeRR();
	}
	else{
		cout << "CPU " << cpuID << ": UNKNOWN SCHEDULING ALGORITHM: " << scheduler << ".\n";
	}	

	for(vector<Process>::iterator i = processes.begin(); i != processes.end(); ++i){
		if(i->checkStatus() != "COMPLETE"){
			processComplete = false;
		}
	}
	for(vector<Core>::iterator i = cores.begin(); i != cores.end(); ++i){
		if(i->getStatus() != "READY"){
			coreComplete = false;
		}
	}
	++cycle;

	if(coreComplete && processComplete == true){
		string msg = "Simulator ended for " + scheduler;
		printMsg(msg);
	}
	return coreComplete && processComplete;
}