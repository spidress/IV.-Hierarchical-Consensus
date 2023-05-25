#include "load.h"

int loadWorker()
{
	int iWorker;
	ifstream fWorker("Worker.txt");
	if(!fWorker)
	{
		fWorker.close();
		printf("cannot open Worker.txt\n");
		return -1;
	}
	fWorker >> iWorker;
	fWorker.close();
	return iWorker;
}

bool loadGroup(string &strWorkerDetail)
{
	ifstream fWorker("Group.txt");
	if(!fWorker)
	{
		fWorker.close();
		printf("cannot open Group.txt\n");
		return false;
	}
	getline(fWorker, strWorkerDetail);
	//fWorker >> iWorker;
	fWorker.close();
	return true;
}

bool loadAllWorkers(map<int, string> &g_mWorkerDetails)
{
	//match IP and port with worker_id
	string strWorkerDetail, strIP;
	int iWorkerTmp, iPort;
	bool bWork = false;
	//load workers' details
	ifstream fWorkers("Workers.txt");

	if(!fWorkers)
	{
		fWorkers.close();
		printf("cannot open Workers.txt\n");
		return false;
	}

	while(getline(fWorkers, strWorkerDetail))
	{
		stringstream ss(strWorkerDetail);
		ss >> iWorkerTmp >> strIP >> iPort >> bWork;
		g_mWorkerDetails.insert(pair<int, string>(iWorkerTmp, strWorkerDetail));
	}
	fWorkers.close();
	return true;
}

bool loadWorkers(map<int, string> &g_mWorkerDetails)
{
	//match IP and port with worker_id
	string strWorkerDetail, strIP;
	int iWorkerTmp, iPort;
	bool bWork = false;
	//load workers' details
	ifstream fWorkers("Workers.txt");

	if(!fWorkers)
	{
		fWorkers.close();
		printf("cannot open Workers.txt\n");
		return false;
	}

	while(getline(fWorkers, strWorkerDetail))
	{
		stringstream ss(strWorkerDetail);
		ss >> iWorkerTmp >> strIP >> iPort >> bWork;
		if(bWork)
			g_mWorkerDetails.insert(pair<int, string>(iWorkerTmp, strWorkerDetail));
	}
	return true;
}

string loadWorkerDetail(int iWorker)
{
	if(!loadAllWorkers(g_mWorkerDetails)) return "";
	if(g_mWorkerDetails.size() == 0) return "";
	else return g_mWorkerDetails[iWorker];
}

bool loadWorkerDetail(int iWorker, string &strIP, int &iPort, bool &bWork)
{
	if(!loadAllWorkers(g_mWorkerDetails)) return false;
	if(g_mWorkerDetails.size() == 0) return false;
	if(g_mWorkerDetails.find(iWorker) == g_mWorkerDetails.end()) return false;
	stringstream ssDetail(g_mWorkerDetails[iWorker]);
	ssDetail >> iWorker >> strIP >> iPort >> bWork;
	return true;
}

double loadTime()
{
	double process_time;
#ifdef _WIN32
	//DWORD now;
	//now = GetTickCount(); //从操作系统启动经过的毫秒数
	//cout << "Time = " << now << "ms\n ";
	SYSTEMTIME st = { 0 };  
	GetLocalTime(&st);  //获取当前时间 可精确到ms
	process_time = ((st.wHour * 60 + st.wMinute) *60 + st.wSecond)*1000 + st.wMilliseconds;
#else
    struct timeval tv; 
    struct timezone tz; 

	/* 获取时间，理论到us */
	gettimeofday(&tv, &tz);
	double dSec = tv.tv_sec * 1000;
	double dMillSec = tv.tv_usec / 1000;
//	printf("second = %.0lf, millsecond = %.0lf\n", dSec, dMillSec);
	process_time = dSec + dMillSec;
	//process_time = now.tv_sec*1000 + now.tv_usec/1000;

	//time_t tt;
	//time( &tt );
	////tt = tt + 8*3600;  // transform the time zone
	//tm* t= gmtime( &tt );
	////cout << tt << endl;
	//
	//process_time = (t->tm_hour * 60 + t->tm_min) *60 + t->tm_sec;
#endif
	return process_time;
}