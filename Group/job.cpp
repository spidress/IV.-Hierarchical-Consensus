#include "job.h"

#define RETRY 100000000

void printWorkers(map<int, vector<Tx*>> t)
{ 
	for(map<int, vector<Tx*>>::iterator it = t.begin() ;it != t.end();it++)
	{
		printf("worker: %d\n", it->first);
		vector<Tx*> t1 = it->second;
		for(vector<Tx*>::iterator it1 = t1.begin(); it1 != t1.end(); it1++)
		{
			(*it1)->print();
			(*it1)->writeFile("chain.log");
		}
	}
}

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
		it->second->writeFile("tip.log");
	}
}

bool newJob(int pk, int iFrom, string strWorkers)
{
	if(g_tasks.findTx(pk))
		return true;

	bool bNeedWork = false;
	string strAction = "newTask";
	string strWorker;
	set<int> setWorkers;
	if(strWorkers.find("All") == string::npos)
	{
		stringstream ssDetail(strWorkers);
		while(ssDetail >> strWorker)
		{
			setWorkers.insert(atoi(strWorker.c_str()));
		}
		if(setWorkers.find(g_iWorker) != setWorkers.end())
			bNeedWork = true;
	}
	else
	{
		bNeedWork = true;
	}

	//if(!sendMsg(EXCL, iFrom, strAction, pk, strWorkers, false))
//	if(!sendMsg(FRIEND, iFrom, strAction, pk, "", strWorkers, false))
//		return false;

	g_num_mutex.lock();
	if(!g_tasks.newTask(pk, strWorkers))
	{
		g_num_mutex.unlock();
		return false;
	}
	g_num_mutex.unlock();

	string strIP;
	int iPort;
	bool bWork;
	if(!loadWorkerDetail(g_iWorker, strIP, iPort, bWork)) return false;
	bNeedWork = bNeedWork & bWork;

	string strFile = to_string(pk) + ".dat";
	ifstream fDat;
	int iRetry = 0;
	bool bOpen = false;
	while (iRetry < 60)
	{
		fDat.open(strFile.c_str(), ios::in);
		if(!fDat.is_open())
		{
			fDat.close();
			int iSleepTime = 1000;
#ifdef _WIN32
			Sleep(iSleepTime);
#else
			usleep(iSleepTime*1000);
#endif
			iRetry++;
			continue;
		}
		else
		{
			bOpen = true;
			break;
		}
	}
	if(!bOpen)
	{
		printf("cannot open %s\n", strFile.c_str());
		return false;
	}

	if(strWorkers.find("All") != string::npos)
	{
		string strContent;
		fDat >> strContent;
		//if(!sendMsg(EXCL, iFrom, "data", pk, strContent, false))
		//if(!sendMsg(EXCL, iFrom, "data", pk, strContent,"", false))
		//	return false;
	}
	fDat.close();

	g_num_mutex.lock();
	if(!g_TaskStack.write(pk))
	{
		printf("g_TaskStack write error!");
		return false;
	}
	g_num_mutex.unlock();

	if(bNeedWork)
	{
		if(!startJob()) return false;
	}
	return true;
}

bool queryJob(int iPK, int iFrom)
{
	string strAction = "recSolution";
	Tx* tip = g_tasks.findTx(iPK, g_iWorker);

	if(tip == NULL)
	{
		printf("%d not found\n", iPK);
	}
	else
	{
		string strMsg = to_string(g_iWorker) + " " + to_string(tip->getSolution()) + " " + to_string(tip->getHeight());
		set<Tx*> preWorkers = tip->getpreWorkers();
		for(set<Tx*>::iterator it = preWorkers.begin(); it != preWorkers.end(); it++)
		{
			strMsg = strMsg + " " + to_string((*it)->getWorker());
		}

		if(!sendMsg(to_string(iFrom), strAction, iPK, strMsg, false))
			return false;
	}
	return true;
}

bool recBlock(int iPK, int iFrom, string strMsg)
{
	map<int, int> mWorkers;        //worker,height
	set<int> sNonTips;
	map<int, int> mTips;           //worker,height
	map<int, int> mSolutions;      //worker, solution
	map<int, Tx*> longTips = g_tasks.getLongTips();
	if(longTips.find(iPK) != longTips.end())
		return true;

	int iMaxHeight = 0;
	int iWinTip = 0;
	set<Tx*> tips;

	int iWorker;
	int iSolution;
	int iHeight;
	stringstream ssMsg(strMsg);
	while (ssMsg >> iWorker >> iSolution >> iHeight)
	{
		vector<int> vPreWorkers;
		int iPreWorker;
		while(ssMsg >> iPreWorker)
		{
			if(iPreWorker == CROWDPLATFORM - 1)
				break;
			vPreWorkers.push_back(iPreWorker);
		}
		string strTmp;
		strTmp = "recSolution " +
			to_string(iPK) + " " + 
			to_string(iFrom) + " " +
			to_string(iWorker) + " " +
			to_string(iSolution) + " " + 
			to_string(iHeight);
		mWorkers.insert(make_pair(iWorker, iHeight));
		mSolutions.insert(make_pair(iWorker, iSolution));
		for(vector<int>::iterator it = vPreWorkers.begin();
			it != vPreWorkers.end();
			it++)
		{
			strTmp += " " + to_string(*it);
			sNonTips.insert(*it);
		}

		if(!g_RecStack.is_full())
			g_RecStack.write(strTmp);
	}
	recJob(false);

	for(map<int, int>::iterator it = mWorkers.begin();
		it != mWorkers.end();
		it++)
	{
		if(sNonTips.find(it->first) == sNonTips.end())
			mTips[it->first] = it->second;
	}

	for(map<int, int>::iterator it = mTips.begin();
		it != mTips.end();
		it++)
	{
		if(iMaxHeight < it->second)
		{
			iMaxHeight = it->second;
			iWinTip = it->first;
		}
	}

	//link all Txs with same solution
	if(iMaxHeight > 0)
	{
		string strTmp;
		strTmp = to_string(g_iWorker) + " " +
			to_string(mSolutions[iWinTip]) + " " + 
			to_string(iMaxHeight + 1);
		for(map<int, int>::iterator it = mTips.begin();
			it != mTips.end();
			it++)
		{
			strTmp += " " + to_string(it->first);
		}
		string strTmp1 = "recSolution " +
			to_string(iPK) + " " + 
			to_string(g_iWorker) + " " + 
			strTmp;
		printf("%s\n", strTmp1.c_str());
		if(!g_RecStack.is_full())
			g_RecStack.write(strTmp1);
		recJob(false);
		
		string strBlocks = strMsg + " " + strTmp + " -2";
		if(!sendMsg(ORDER, g_iWorker, "report", iPK, strBlocks, "All", false, g_iWorker))
			return false;
	}
	return true;
}

bool disBlock(int iPK, int iFrom, string strMsg)
{
	sendMsg(GPs, iFrom, "disBlock", iPK, strMsg, "All", false, iFrom);

	map<int, int> mWorkers;        //worker,height
	set<int> sNonTips;
	map<int, int> mTips;           //worker,height
	map<int, int> mSolutions;      //worker, solution
	map<int, Tx*> longTips = g_tasks.getLongTips();

	int iMaxHeight = 0;
	int iWinTip = 0;
	set<Tx*> tips;

	int iWorker;
	int iSolution;
	int iHeight;
	stringstream ssMsg(strMsg);
	while (ssMsg >> iWorker >> iSolution >> iHeight)
	{
		vector<int> vPreWorkers;
		int iPreWorker;
		while(ssMsg >> iPreWorker)
		{
			if(iPreWorker == CROWDPLATFORM - 1)
				break;
			vPreWorkers.push_back(iPreWorker);
		}
		string strTmp;
		strTmp = "recSolution " +
			to_string(iPK) + " " + 
			to_string(iFrom) + " " +
			to_string(iWorker) + " " +
			to_string(iSolution) + " " + 
			to_string(iHeight);
		mWorkers.insert(make_pair(iWorker, iHeight));
		mSolutions.insert(make_pair(iWorker, iSolution));
		for(vector<int>::iterator it = vPreWorkers.begin();
			it != vPreWorkers.end();
			it++)
		{
			strTmp += " " + to_string(*it);
			sNonTips.insert(*it);
		}

		if(!g_RecStack.is_full())
			g_RecStack.write(strTmp);
	}
	recJob(false);
	//sendMsg(WORKER, iFrom, "disBlock", iPK, strMsg, "All", false, iFrom);

	return true;
}

bool recJob(bool bSend)
{
	Fifo<string> RecStack(FIFO_SIZE);
	string strDetail;
	while (!g_RecStack.is_empty())
	{
		g_num_mutex.lock();
		if(!g_RecStack.read(&strDetail))
		{
			g_num_mutex.unlock();
			continue;
		}
		g_num_mutex.unlock();
		int pk;
		string strAction;
		int iFrom;
		stringstream ssDetail(strDetail);
		int iWorker = CROWDPLATFORM;
		int iSolution = 0;
		int iHeight = 0;
		int preWorker;

		set<int> setpreWorkers;
		ssDetail >> strAction >> pk >> iFrom >> iWorker >> iSolution >> iHeight;
		string strMsg = to_string(iWorker) + " " + to_string(iSolution) + " " + to_string(iHeight);
		while(ssDetail >> preWorker)
		{
			setpreWorkers.insert(preWorker);
			strMsg = strMsg + " "  + to_string(preWorker);
		}

		if(bSend && (g_tasks.findTx(pk, iWorker) == NULL))
		{
			//if(!sendMsg(EXCL, iFrom, strAction, pk, strMsg, false))
			string strFriends;
			if(g_tasks.findTx(pk, CROWDPLATFORM) == NULL) strFriends = "";
			else strFriends = g_tasks.findTx(pk, CROWDPLATFORM)->getRemarks();

			if(!sendMsg(WORKER, iFrom, strAction, pk, strMsg, strFriends, false, iWorker))
				return false;
		}

		g_num_mutex.lock();
		if(!g_tasks.recTx(pk, iWorker, iSolution, iHeight, setpreWorkers))
		{
			for(set<int>::iterator it = setpreWorkers.begin();
				it != setpreWorkers.end();
				it++)
			{
				if((g_tasks.findTx(pk, *it) == NULL) && !g_NotFound.is_full())
				{
					string strNotFound = "query " + to_string(pk) + " " + to_string(g_iWorker) + " " + to_string(*it);
					if(!g_NotFound.is_full())
						g_NotFound.write(strNotFound);
				}
			}
			RecStack.write(strDetail);
		}
		g_num_mutex.unlock();
	}
	while(!g_NotFound.is_empty())
	{
		g_NotFound.read(&strDetail);
		stringstream ss(strDetail);
		string strAction;
		int pk, iWorker, iFrom;
		ss >> strAction >> pk >> iFrom >> iWorker;

		if(!sendMsg(to_string(iWorker), strAction, pk, to_string(iWorker), false))
		{
			printf("sendMsg(%d, %s, %d, %s, false) error\n", iWorker, strAction.c_str(), pk,  to_string(iWorker).c_str());
		}
	}
	while(!RecStack.is_empty() && !g_RecStack.is_full())
	{
		RecStack.read(&strDetail);
		g_RecStack.write(strDetail);
	}
	return true;
}

bool startJob()
{
	string strPuzzle;
	int iSolution = 0;
	int time = 0;

	int pk = 0;
	while(g_TaskStack.read(&pk))
	{
		string strFile = "fake.dat";
		ifstream fFake;
		fFake.open(strFile.c_str(), ios::in);
		if(!fFake.is_open())
		{
			fFake.close();
			//honest action
			string strFile = to_string(pk) + ".dat";
			ifstream fDat(strFile);
			if(!fDat)
			{
				fDat.close();
				printf("cannot open %s\n", strFile.c_str());
				return false;
			}
			fDat >> strPuzzle;
			fDat.close();

//			int iSleepTime = g_iWorker - CROWDPLATFORM;
//#ifdef _WIN32
//			Sleep(iSleepTime);
//#else
//			usleep(iSleepTime*1000);
//#endif
			while(iSolution < RETRY)
			{
				string strSolution = to_string(iSolution);
				Md5Encode encode_obj;
				string strTmpSolution = encode_obj.Encode(strSolution);
				if (strPuzzle == strTmpSolution)
				{
					printf("solution is %d\n", iSolution);
					break;
				}
				iSolution++;
			}
		}
		else
		{
			//malicious action
			iSolution = 0;
		}

		//get preTx
		int height;
		set<int> setpreWorkers;

		g_num_mutex.lock();
		//worker solution height preWorker
		string strFriends;
//		bool bDone = false;
		string strMsg2Servers = g_tasks.addTx(pk, 
			                                  g_iWorker,
											  iSolution, 
											  height,
											  setpreWorkers,
											  strFriends);
//		if(bDone)
//		{
//			string strBlocks = g_tasks.getBlock(pk);
//			if(!sendMsg(ALL, g_iWorker, "recBlock", pk, strBlocks, strFriends, false, g_iWorker))
//				return false;
//		}
		g_num_mutex.unlock();
		if(strMsg2Servers ==  "")
			return false;

		//announce to all workers
		//map<int, string> g_mWorkerDetails;
		//if(!loadAllWorkers(g_mWorkerDetails)) return false;
		//for (map<int, string>::iterator it = g_mWorkerDetails.begin(); it != g_mWorkerDetails.end(); it++)
		//{
		//	if(it->first == g_iWorker) continue;
		//	stringstream workerDetail(it->second);
		//	int iReceiver;
		//	workerDetail >> iReceiver;
		//	if(!sendMsg(strMsg2Servers, to_string(iReceiver), false))
		//	{
		//		printf("send to worker %d msg %s error\n", iReceiver, strMsg2Servers.c_str());
		//		continue;
		//	};
		//}
		//if(!sendMsg(EXCL, CROWDPLATFORM, "recSolution", pk, strMsg2Servers,  false))
		if(!sendMsg(WORKER, g_iWorker, "recSolution", pk, strMsg2Servers, strFriends, false))
		{
			continue;
		}
	}
	return true;
}

bool printJob()
{
#ifdef _WIN32
	system("del chain.log");
	system("del tip.log");
#else
	system("rm chain.log");
	system("rm tip.log");
#endif
	printf("\nprintWorkers\n");
	printWorkers(g_tasks.getWorkers());
	printf("\n\nprintLongTips\n");
	printLongTips(g_tasks.getLongTips());
	return true;
}