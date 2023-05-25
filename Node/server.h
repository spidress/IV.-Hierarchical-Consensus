#ifndef __SERVER__
#define __SERVER__
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fstream>
#include <sstream>

#ifdef _WIN32
	#include <windows.h>
    #include <winsock2.h> 
    #pragma comment(lib,"ws2_32.lib")  //æ≤Ã¨º”‘ÿws2_32.lib
#else
    #include<unistd.h>
    #include<sys/types.h>
    #include<sys/socket.h>
    #include<netinet/in.h>
#endif

#define MAXLINE 4096

//using namespace std;

#include "job.h"
#include "negotiate.h"
#include "load.h"
#include "fifo.h"

int startServer();

extern Fifo<string> g_RecStack;
extern int iWorker;
extern set<string> g_setMsg;

#endif