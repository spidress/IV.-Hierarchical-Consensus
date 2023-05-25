#include "chain.h"

void printTips(set<Tx*> t)
{
	for(set<Tx*>::iterator it = t.begin() ;it != t.end();it++)
	{
		(*it)->print();
		printTips((*it)->getpreWorkers());
	}
}

void printLongTips(map<int, Tx*> t)
{
	for(map<int, Tx*>::iterator it = t.begin(); it != t.end(); it++)
	{
		it->second->print();
		printTips(it->second->getpreWorkers());
	}
}

void WorkersRecord(Tx* t, map<int, int> &RepRank)
{
	if((t->getWorker() == 5) || (t->getWorker() == 2))
		cout << "";
	if(t->getWorker() != 0)
	{
		RepRank[t->getWorker()] = 0;
		//			if(RepRank[(*it)->getWorker()] > 1) return;   //already count

		set<Tx*> sSucWorkers = t->getsucWorkers();
		//RepRank[t->getWorker()] += sSucWorkers.size();
		for(set<Tx*>::iterator it1 = sSucWorkers.begin(); it1 != sSucWorkers.end(); it1++)
		{
			RepRank[t->getWorker()] += RepRank[(*it1)->getWorker()];
		}
		RepRank[t->getWorker()] += sSucWorkers.size();

		set<Tx*> sPreWorkers = t->getpreWorkers();
		for(set<Tx*>::iterator it = sPreWorkers.begin(); it != sPreWorkers.end(); it++)
			WorkersRecord(*it, RepRank);
	}
}

void WorkersRecord(map<int, Tx*> t, map<int, int> &RepRank)
{
	for(map<int, Tx*>::iterator it = t.begin(); it != t.end(); it++)
	{
		map<int, int> t_RepRank = RepRank;
		RepRank.clear();
		if(it->second->getWorker() != 0)
		{
			WorkersRecord(it->second, RepRank);
		}
		for(map<int, int>::iterator it1 = t_RepRank.begin(); it1 != t_RepRank.end(); it1++)
		{
			RepRank[it1->first] = RepRank[it1->first] + t_RepRank[it1->first];
		}
	}
}

void rankWorkers(Tasks &tasks, vector<pair<int, int>> &t_vecRank)  //worker, SubWeight
{
	map<int, int> RepRank;	//worker, count
	map<int, vector<Tx*>> mWorkerTx = tasks.getWorkers();
	//for(map<int, vector<Tx*>>::iterator it = mWorkerTx.begin(); it != mWorkerTx.end(); it++)
	//	RepRank.insert(pair<int, int>(it->first, 0));  //initial

	WorkersRecord(tasks.getLongTips(), RepRank);//rank workers in LongTips
	for(map<int, int>::iterator it = RepRank.begin(); it != RepRank.end(); it++)
	{
		t_vecRank.push_back(*it);
	}
	pair<int, int> tmp;
	for(int i = 0; i < t_vecRank.size(); i++)
	{
		for(int j = i + 1; j < t_vecRank.size(); j++)
		{
			if(t_vecRank[i].second < t_vecRank[j].second)
			{
				tmp = t_vecRank[i];
				t_vecRank[i] = t_vecRank[j];
				t_vecRank[j] = tmp;
			}
		}
	}
}

void printWorkers(map<int, vector<Tx*>> t)
{ 
	for(map<int, vector<Tx*>>::iterator it = t.begin() ;it != t.end();it++)
	{
		printf("worker: %d\n", it->first);
		vector<Tx*> t1 = it->second;
		for(vector<Tx*>::iterator it1 = t1.begin() ;it1 != t1.end();it1++)
		{
			(*it1)->print();
		}
	}
}

bool processLine(Tasks &tasks, string line, int iWorker, map<int, map<string, int>> mWorkers)
{
	string cmd;
	int pk;
	int worker;
	int solution;
	int height;
	string preWorkers;
	int workerCnt = 0;
	int preWorker;
	set<int> setpreWorkers;
	map<string, int> mIP;
	char clientCmd[4096];
	double process_time;

#ifdef _WIN32
	//DWORD now;
	//now = GetTickCount(); //从操作系统启动经过的毫秒数
	//cout << "Time = " << now << "ms\n ";
	SYSTEMTIME st = { 0 };  
	GetLocalTime(&st);  //获取当前时间 可精确到ms
	process_time = (st.wHour * 60 + st.wMinute) *60 + st.wSecond;
#else
	//struct timeval now;

	///* 获取时间，理论到us */
	//gettimeofday(&now, NULL);
	//printf("s: %ld, ms: %ld\n", now.tv_sec, (now.tv_sec*1000 + now.tv_usec/1000));

	time_t tt;
	time( &tt );
	//tt = tt + 8*3600;  // transform the time zone
	tm* t= gmtime( &tt );
	//cout << tt << endl;

	process_time = (t->tm_hour * 60 + t->tm_min) *60 + t->tm_sec;
#endif

	if(line.length() > 0)
	{
		setpreWorkers.clear();
		istringstream sline(line); 
		sline >> cmd >> pk;

		if (cmd == "newTask")
		{
			sline >> workerCnt;
			if(!tasks.newTask(pk, workerCnt, process_time))
				return false;
			else return true;
		}
		if (cmd == "addTx")
		{
			sline >> worker >> solution;

			if(!tasks.addTx(pk, worker, solution, height, setpreWorkers, process_time))
				return false;
			else
			{
				//cout << "mWorkers size" << mWorkers.size() << endl;
				for(map<int, map<string, int>>::iterator it0 = mWorkers.begin(); it0 != mWorkers.end(); it0++)
				{
					if(it0->first == iWorker) continue;
					ostringstream ssendLine;
					ssendLine << "sendTx "<< pk << " " << worker << " " << solution << " " << height << " ";
					//cout << "sendTx "<< pk << " " << worker << " " << solution << " " << height << " ";
					for(set<int>::iterator it = setpreWorkers.begin(); it != setpreWorkers.end(); it++)
					{
						ssendLine << *it << " ";
						//cout << *it << " ";
					}
					//cout << endl;
					string parameter = ssendLine.str();
					mIP = it0->second;
					//cout << "mIP.size" << mIP.size() << endl;
					for(map<string, int>::iterator it1 = mIP.begin(); it1 != mIP.end(); it1++)
					{
#ifdef _WIN32
						sprintf(clientCmd, "debug\\client.exe %s %d %s", it1->first.c_str(), it1->second, parameter.c_str());
						system(clientCmd);
#else
						sprintf(clientCmd, "./client.exe %s %d %s>./client.log", it1->first.c_str(), it1->second, parameter.c_str());
						system(clientCmd);
#endif
					}
				}
				return true;
			}
		}
		if (cmd == "recTx")
		{
			sline >> worker >> solution >> height;
			while(sline >> preWorker)
				setpreWorkers.insert(preWorker);

			if(!tasks.recTx(pk, worker, solution, height, setpreWorkers, process_time))
				return false;
			else return true;
		}
	}
}

int add2Chain(char* szBuff, string strWorkerDetail)
{
	string line(szBuff);
	Tasks tasks;
	vector<string> errLine;

	string strIP;
	int iWorker, iPort;
	bool bWork = false;

	stringstream ss(strWorkerDetail);
	ss >> iWorker >> strIP >> iPort >> bWork;
//		if(!processLine(tasks, line, iWorker, mWorkers))
//			errLine.push_back(line);

//			//rank workers
//			vector<pair<int, int>> vecRank;  //worker, rank
//			rankWorkers(tasks, vecRank);
//
//			for(int i = 0; i < vecRank.size(); i++)
//			{
//				if(vecRank[i].second > 0)
//				{
//					if(i == 0)
//					{
//						sprintf(clientCmd, "echo %d %d > rank.txt", vecRank[i].first, vecRank[i].second);
//					}
//					else
//					{
//						sprintf(clientCmd, "echo %d %d >> rank.txt", vecRank[i].first, vecRank[i].second);
//					}
//					system(clientCmd);
//				}
//			}

	cout << "reprocess errLine" << endl;
	for(vector<string>::iterator it = errLine.begin(); it != errLine.end(); it++)
	{
		printf("errLine:\n");
//		if(!processLine(tasks, *it, iWorker, mWorkers))
//			cout << *it << endl;
	}

	//stop server
//	map<string, int> mIP = mWorkers[iWorker];
//	for(map<string, int>::iterator it = mIP.begin(); it != mIP.end(); it++)
//	{
//#ifdef _WIN32
//		sprintf(clientCmd, "debug\\client.exe %s %d stop", it->first.c_str(), it->second);
//		cout << clientCmd << endl;
//#else
//		sprintf(clientCmd, "./client.exe %s %d stop>./client.log", it->first.c_str(), it->second);
//		system(clientCmd);
//#endif
//	}

//	//printf("printTips\n");
//	//printTips(tasks.getTips());
//	printf("\nprintLongTip\n");
//	printLongTips(tasks.getLongTips());
//	printf("\nprintWorkers\n");
//	printWorkers(tasks.getWorkers());
//
	return 0;
}
