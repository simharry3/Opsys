#include "memDevice.h"

using namespace std;
int main(int argc, char* argv[]){
	int fpl = 32;
	string testInitializer = "Q 100 0/100 200/100";
	memDevice bank1(256, 1, "Next-Fit", 32);
	bank1.loadProcesses(argv[1]);
	//bank1.printProcesses();
	bank1.updateFreeSpace();
	//bank1.printFreeSpace(fpl);
	
	int n = 560;
	for(int i = 0; i < n; ++i){
		bank1.cycleMemDevice();
		//bank1.printMem(fpl);
	}		
	// bank1.printFreeSpace(fpl);
	// bank1.defrag();
	// cout << "//////// DEFRAG ////////" << endl;
	// bank1.printMem(fpl);
	// bank1.printFreeSpace(fpl);

}