
using namespace std;

class dataEntry{
public:
	void setUserProcess();
	void setStart();
	void setEnd();

	char getUserProcess(){return userProcess;}
	int getStart(){return start;}
	int getEnd(){return end;}
private:
	char userProcess;
	int start;
	int end;
}
class memDevice{
public:
	int size;
	vector<dataEntry> data;
}