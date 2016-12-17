////////////////////////////////////////////////////////////
//Computer Operating Systems Project 2: Memory Management
//
//Written By: Clayton Rayment
//RCSID: raymec
//RIN: 661133772
///////////////////////////////////////////////////////////

#include "memDevice.h"
#include "pageReference.h"

using namespace std;
int main(int argc, char* argv[]){

	//Here you can set the frames per line for memoryDevice output:
	int fpl = 32;

	//Main test block for memoryDevices:
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


	//Main test block for virtualMemoryDevices:
	virtualMemoryDevice bank6("OPT", argv[2]);
	while(!bank6.cycleMemDevice());

	//When you construct a virtualMemoryDevice, you can leave off the frame size, and it defaults to 3,
	//otherwise, it will be whatever value you set as the last argument:
	virtualMemoryDevice bank7("LRU", argv[2], 3);
	while(!bank7.cycleMemDevice());

	virtualMemoryDevice bank8("LFU", argv[2]);
	while(!bank8.cycleMemDevice());
}


/////////////////////////////////////////////
//------------------------------------------
//		     _   _______	 
//     _-_- |_/\__\___\___
//=  _=_-_  / ,-. -|-  ,-.`-.  =	  =	   =
//     _-_- `( o )----( o )-'
//            `-'      `-'
// WHEN   LIFE   THROWS   YOU  CURVES
//------------------------------------------
//		 H I T   T H E   A P E X
////////////////////////////////////////////