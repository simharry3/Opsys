#include "memDevice.h"
#include "pageReference.h"

using namespace std;
int main(int argc, char* argv[]){
	int fpl = 32;
	string testInitializer = "Q 100 0/100 200/100";
	memDevice bank1(256, 1, "Contiguous", "Next-Fit", fpl);
	bank1.loadProcesses(argv[1]);
	while(!bank1.getStatus()){
		bank1.cycleMemDevice();
	}

	memDevice bank2(256, 1, "Contiguous", "Best-Fit", fpl);
	bank2.loadProcesses(argv[1]);
	bank2.updateFreeSpace();
	while(!bank2.getStatus()){
		bank2.cycleMemDevice();
	}

	memDevice bank3(256, 1, "Contiguous", "Worst-Fit", fpl);
	bank3.loadProcesses(argv[1]);
	bank3.updateFreeSpace();
	while(!bank3.getStatus()){
		bank3.cycleMemDevice();
	}

	memDevice bank5(256, 1, "Non-Contiguous", "First-Fit", fpl);
	bank5.loadProcesses(argv[1]);
	bank5.printMem();
	while(!bank5.getStatus()){
		bank5.cycleMemDevice();
	}

	virtualMemoryDevice bank6("OPT", argv[2]);
	while(!bank6.cycleMemDevice());

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