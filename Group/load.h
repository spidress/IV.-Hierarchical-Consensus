#ifndef __LOAD__
#define __LOAD__
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/time.h>
#endif
using namespace std;

extern map<int, string> g_mWorkerDetails;
extern int g_iWorker;

bool loadWorker(string &strWorkerDetail);
bool loadAllWorkers(map<int, string> &g_mWorkerDetails);
bool loadWorkers(map<int, string> &g_mWorkerDetails);
bool loadAllGPs(map<int, string> &g_mGroups);
string loadWorkerDetail(int iWorker);
bool loadWorkerDetail(int iWorker, string &strIP, int &iPort, bool &bWork);
double loadTime();
#endif