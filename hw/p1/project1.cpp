#include <iostream>
#include <fstream>
#include "processor.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////
//Written by: Clayton Rayment
//This program simulates a CPU and scheduling algorithm for different scheduling algorithms.
//While there are more efficient ways to implement this, this simulator steps through each step of the
//CPU simulation cycle by cycle to emulate a real processor. This is done for extensibility purposes.
//////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
int main(int argc, char* argv[]){
	if(argc < 3){
		cout << "INCORRECT NUMBER OF ARGUMENTS: \"./a.out <INPUT.txt> <OUTPUT.txt>\"\n";
		exit(EXIT_FAILURE);
	}

	ofstream output;
	output.open(argv[2]);

	CPU cpu1(1, 1, 8, 99999, "FCFS");
	cpu1.loadProcesses(argv[1]);
	while(!cpu1.executeCycle());
	cpu1.printStatistics(&output);

	cout << "\n";

	CPU cpu2(1, 1, 8, 99999, "SJF");
	cpu2.loadProcesses(argv[1]);
	while(!cpu2.executeCycle());
	cpu2.printStatistics(&output);


	cout << "\n";

	CPU cpu3(1, 1, 8, 84, "RR");
	cpu3.loadProcesses(argv[1]);
	while(!cpu3.executeCycle());
	cpu3.printStatistics(&output);

	output.close();
}