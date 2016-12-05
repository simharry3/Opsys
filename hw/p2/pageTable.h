#include "process.h"
#include <utility>

using namespace std;

class pageTable{
public:
	void storeProcess(process* );
	void cycleDevice();

private:
	list<pair<process, dataEntry> > table;
};