//���߳�
#include <thread>
#include <mutex>

#include "server.h"
#include "client.h"
#include "job.h"
#include "fifo.h"
#include "load.h"
using namespace std;

#define FIFO_SIZE 100
int g_iWorker;
Tasks g_tasks;
map<int, string> g_mWorkerDetails;	//worker, IP, port
std::mutex g_num_mutex;
set<string> g_setMsg;
Fifo<int> g_TaskStack(FIFO_SIZE);
Fifo<string> g_SolutionStack(FIFO_SIZE);
Fifo<string> g_RecStack(FIFO_SIZE);
Fifo<string> g_NotFound(FIFO_SIZE);





