#ifndef __TX__
#define __TX__
#include <stdio.h>
#include <set>
#include <map>
#include <cstring>
#include <vector>
#include <sstream>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#include "load.h"
#include "tx.h"
#include "fifo.h"

using namespace std;
extern Fifo<string> g_NotFound;
extern int g_iWorker;

#define CROWDPLATFORM (-1)
#define INIT_CREDIBILITY 0.49
class Tx
{
	int TP_public_key;
	int Worker;
	int E_solution;
	int Height;
	int WorkerCnt;
	int PreWeight;
	double process_time;
	Tx* preTask;
	set<Tx*> preWorkers;
	Tx* sucTask;
	set<Tx*> sucWorkers;
	char Signature[36];
	char CA[36];
	char Remarks[512];

public:
	Tx(int t_TP_public_key, int t_Worker, int t_E_solution)
	{
		TP_public_key = t_TP_public_key;
		Height = 0;
		WorkerCnt = 0;
		PreWeight = 0;
		Worker = t_Worker;
		preTask = NULL;
		preWorkers.clear();
		sucTask = NULL;
		sucWorkers.clear();
		E_solution = t_E_solution;
		process_time = loadTime();
		memset(Signature, 0, sizeof(Signature));
		memset(CA, 0, sizeof(CA));
	}

	Tx(int t_TP_public_key, int t_Worker, int t_E_solution, int t_Height)
	{
		TP_public_key = t_TP_public_key;
		Height = t_Height;
		WorkerCnt = 0;
		PreWeight = 0;
		Worker = t_Worker;
		preTask = NULL;
		preWorkers.clear();
		sucTask = NULL;
		sucWorkers.clear();
		E_solution = t_E_solution;
		process_time = loadTime();
		memset(Signature, 0, sizeof(Signature));
		memset(CA, 0, sizeof(CA));
	}

	Tx(int t_TP_public_key, int t_workerCnt)
	{
		TP_public_key = t_TP_public_key;
		Height = 0;
		WorkerCnt = t_workerCnt;
		PreWeight = 0;
		Worker = CROWDPLATFORM;
		preTask = NULL;
		preWorkers.clear();
		sucTask = NULL;
		sucWorkers.clear();
		E_solution = 0;
		process_time = loadTime();
		memset(Signature, 0, sizeof(Signature));
		memset(CA, 0, sizeof(CA));
	}

	Tx(int t_TP_public_key, string t_strWorkers)
	{
		TP_public_key = t_TP_public_key;
		Height = 0;
		PreWeight = 0;
		Worker = CROWDPLATFORM;
		preTask = NULL;
		preWorkers.clear();
		sucTask = NULL;
		sucWorkers.clear();
		E_solution = 0;
		process_time = loadTime();
		memset(Signature, 0, sizeof(Signature));
		memset(CA, 0, sizeof(CA));
		strncpy(Remarks, t_strWorkers.c_str(), sizeof(Remarks));

		int t_workerCnt = 0;
		stringstream ssWorkers(t_strWorkers);
		int tWorker;
		while(ssWorkers >> tWorker)
		{
			t_workerCnt++;
		}
		WorkerCnt = t_workerCnt;
	}

	void point2Task(Tx *b)
	{
		TP_public_key = b->TP_public_key;
		b->sucWorkers.insert(this);
		preWorkers.insert(b);
		Height = b->Height + 1;
	}

	int getSucWeight()
	{
		int sucWeight = 0;
		for(set<Tx*>::iterator it = sucWorkers.begin(); it!= sucWorkers.end(); it++)
		{
			sucWeight += (*it)->getSucWeight();
			sucWeight++;
		}
		return sucWeight;
	}

	int getTP_public_key()
	{
		return TP_public_key;
	}
	int getSolution()
	{
		return E_solution;
	}
	Tx* getpreTask()
	{
		return preTask;
	}
	set<Tx*> getpreWorkers()
	{
		return preWorkers;
	}
	set<Tx*> getsucWorkers()
	{
		return sucWorkers;
	}
	int getsucWorkersCnt()
	{
		return sucWorkers.size();
	}
	int getWorker()
	{
		return Worker;
	}
	int getHeight()
	{
		return Height;
	}
	void addsucWorkers(Tx* t)
	{
		sucWorkers.insert(t);
	}
	void addpreWorkers(Tx* t)
	{
		preWorkers.insert(t);
	}
	void setHeight(int height)
	{
		Height = height;
	}
	void erasesucWorkers(Tx* t)
	{
		sucWorkers.erase(t);
	}
	int getWorkerCnt()
	{
		return WorkerCnt;
	}
	double getTime()
	{
		return process_time;
	}
	string getRemarks()
	{
		return Remarks;
	}
	void setWorkerCnt(int t_workerCnt)
	{
		WorkerCnt = t_workerCnt;
	}
	void setPreWeight(int t_PreWeight)
	{
		PreWeight = t_PreWeight;
	}
	int getPreWeight()
	{
		return PreWeight;
	}
	void setpreTask(Tx* t)
	{
		preTask = t;
	}
	void setsucTask(Tx* t)
	{
		sucTask = t;
	}
	void print();
	void writeFile(string strFileName);
};


class Tasks
{
	map<int, Tx*> TaskGenesises;	//task, Tx*
	map<int, Tx*> WorkerTips;	//worker, Tx*
	map<int, Tx*> LongTips;	//task, Tx*
	set<Tx*> Tips;
	set<Tx*> allTx;
	map<int, vector<Tx*>> allWorkers;    //worker, Tx*
public:
	Tasks()
	{
		TaskGenesises.clear();
		Tips.clear();
	}

	~Tasks()
	{
		for(set<Tx*>::iterator it = allTx.begin(); it != allTx.end(); it++)
		{
//			(*it)->print();
			delete *it;
		}
	}

	Tx* findTx(int t_TP_public_key, int iWorker = CROWDPLATFORM);

	bool newTask(int t_TP_public_key, int workerCnt);
	bool newTask(int t_TP_public_key, string strWorkers);

// find in preWorker with t_TP_public_key and t_Worker
	bool findWorker(set<Tx*> t, int t_TP_public_key, int t_Worker);

	// find in preWorker with t_TP_public_key, t_Solution and > t_Height, t_preIntWorkers, return t_preWorkers
	bool findTx(set<Tx*> t_txs, int t_TP_public_key, int t_Solution, int t_Height, set<int> t_preIntWorkers, set<Tx*> &t_preWorkers);

//get all Tx that are pre of t_txs
	void getAllPreWorker(set<Tx*> t_txs, set<Tx*> &allTxs)
	{
		for(set<Tx*>::iterator it = t_txs.begin(); it != t_txs.end(); it++)
		{
			allTxs.insert(*it);
			getAllPreWorker((*it)->getpreWorkers(), allTxs);
		}
	}

	bool setWorkerCnt(Tx* &t);

	string addTx(int t_TP_public_key,
		         int t_Worker,
				 int t_Solution,
				 int &t_height,
				 set<int> &t_preIntWorkers,
				 string &strFriends);
	
	string getBlock(int iPK);
	string getTxMsg(Tx* tx);
	bool getBlkMsg(Tx* tx, set<string> &setBlkMsg);
	bool isFinished(int iPK, int iWorkerCnt);

	bool recTx(int t_TP_public_key, int t_Worker, int t_Solution, int t_height, set<int> t_preIntWorkers);

	set<Tx*> getTips()
	{
		return Tips;
	}

	map<int, Tx*> getLongTips()
	{
		return LongTips;
	}

	map<int, Tx*>  getWorkerTips()
	{
		return WorkerTips;
	}

	Tx* getWorkerTips(int iWorker)
	{
		return WorkerTips[iWorker];
	}

	map<int, vector<Tx*>> getWorkers()
	{
		return allWorkers;
	}

	bool getReputations(map<int, double> &mReputations);

	bool getWorkloads(map<int, double> &mWorkloads);
};


#endif