#include "memDevice.h"

using namespace std;
int main(int argc, char* argv[]){
	int fpl = 32;
	string testInitializer = "Q 100 0/100 200/100";
	memDevice bank1(256, 1, "Contiguous", "Next-Fit", 32);
	bank1.loadProcesses(argv[1]);
	bank1.updateFreeSpace();
	while(!bank1.getStatus()){
		bank1.cycleMemDevice();
	}

	memDevice bank2(256, 1, "Contiguous", "Best-Fit", 32);
	bank2.loadProcesses(argv[1]);
	bank2.updateFreeSpace();
	while(!bank2.getStatus()){
		bank2.cycleMemDevice();
	}

	memDevice bank3(256, 1, "Contiguous", "Worst-Fit", 32);
	bank3.loadProcesses(argv[1]);
	bank3.updateFreeSpace();
	while(!bank3.getStatus()){
		bank3.cycleMemDevice();
	}

	memDevice bank4(256, 1, "Non-Contiguous", "Next-Fit", 32);
	bank4.loadProcesses(argv[1]);
	bank4.updateFreeSpace();
	while(!bank4.getStatus()){
		bank4.cycleMemDevice();
	}
	// int n = 560;
	// for(int i = 0; i < n; ++i){
	// 	bank1.cycleMemDevice();
	// 	//bank1.printMem(fpl);
	// }		
	// bank1.printFreeSpace(fpl);
	// bank1.defrag();
	// cout << "//////// DEFRAG ////////" << endl;
	// bank1.printMem(fpl);
	// bank1.printFreeSpace(fpl);

}