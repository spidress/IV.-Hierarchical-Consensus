#ifndef __JOB__
#define __JOB__
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <mutex>
#include <thread>
#include <algorithm>
//#ifdef _WIN32
//#include <windows.h>
//#else
//#include <unistd.h>
//#endif

#include "tx.h"
#include "fifo.h"
#include "md5_encode.h"
#include "client.h"
using namespace std;

#define SLEEPTIME 2

extern int g_iWorker;
extern std::mutex      g_num_mutex;
extern Fifo<int> g_TaskStack;
extern Fifo<string> g_RecStack;
extern Tasks g_tasks;
extern Fifo<string> g_NotFound;

#define FIFO_SIZE 100

bool startJob();
bool newJob(int pk, int iFrom, string strDetail);
bool queryJob(int iPK, int iFrom);
bool recJob(bool bSend);
bool recBlock(int iPK, int iFrom, string strMsg);
bool disBlock(int iPK, int iFrom, string strMsg);
bool printJob();
void printTips(set<Tx*> t);
void printLongTips(map<int, Tx*> t);
#endif