#include <iostream>
#include "memDevice.h"

using namespace std;
int main(int argc, char* argv[]){
	memDevice bank1(256);
	bank1.addEntry('B', 32, 32, 10);
	bank1.addEntry('A', 10, 16, 10);
	bank1.addEntry('C', 128, 64, 10);
	bank1.printMem(16);
	cout << "BEGINNING DEFRAG\n";
	bank1.defrag();
	bank1.printMem(16);
}