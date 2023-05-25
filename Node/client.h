#ifndef __CLIENT__
#define __CLIENT__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <thread>
#include <math.h>
#include <set>
#ifdef _WIN32
  #include <winsock2.h> 
  #pragma comment(lib,"ws2_32.lib")  //æ≤Ã¨º”‘ÿws2_32.lib
#else
  #include<sys/types.h>
  #include<sys/socket.h>
  #include<netinet/in.h>
  #include<arpa/inet.h>
  #include<unistd.h>
#endif
#include<fstream>
#include<sstream>
#include <mutex>
#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#include "load.h"
#include "fifo.h"
#include "client.h"

enum ToWhom
{
	SELF = 0, WORKER = 1, ORDER = 2, GP = 3, GPs = 4
};

#define MAXLINE 4096
using namespace std;

//extern set<string> g_setMsg;
extern int g_iWorker;
extern Fifo<string> g_SolutionStack;
extern std::mutex      g_num_mutex;

int startClient();
bool sendMsg(string strIP, int iPort, string strAction, int iPK, string strDetail);
bool sendMsg(ToWhom iWhom,
			 int iFrom,
			 string strAction,
			 int iPK,
			 string strDetail,
			 string strFriends,
			 bool bConsiderWork,
			 int iExclude = -1);
bool sendMsg(string strWhom, string strAction, int iPK, string strDetail, bool bConsiderWork);

#define SLEEPTIME 2
#define CROWDPLATFORM (-1)

#endif