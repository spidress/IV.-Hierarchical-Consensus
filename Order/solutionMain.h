//∂‡œﬂ≥Ã
#include <thread>
#include <mutex>

#include "tx.h"
#include "server.h"
#include "client.h"
#include "job.h"
#include "fifo.h"
#include "load.h"
using namespace std;

#define FIFO_SIZE 100
Blk *g_blk;
int g_iWorker;
Tasks g_tasks;
map<int, string> g_mGroups;	//worker, IP, port
map<int, string> g_mWorkerDetails;	//worker, IP, port
mutex g_num_mutex;
set<string> g_setMsg;
Fifo<int> g_TaskStack(FIFO_SIZE);
Fifo<string> g_SolutionStack(FIFO_SIZE);
Fifo<string> g_RecStack(FIFO_SIZE);
Fifo<string> g_NotFound(FIFO_SIZE);





