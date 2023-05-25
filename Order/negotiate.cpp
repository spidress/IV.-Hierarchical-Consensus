#include "negotiate.h"

bool selectNodes(vector<int> &vWorker, double dRequirement)
{
	map<int, double> mReputations;
	map<int, double> mWorkloads;
	g_tasks.getReputations(mReputations);
	printf("Reputation:\n");
	for(map<int, double>::iterator it = mReputations.begin();
		it != mReputations.end();
		it++)
	{
		printf("%d %lf\n", it->first, it->second);
	}

	g_tasks.getWorkloads(mWorkloads);
	printf("Workloads:\n");
	for(map<int, double>::iterator it = mWorkloads.begin();
		it != mWorkloads.end();
		it++)
	{
		printf("%d %lf\n", it->first, it->second);
	}

	int iWorker = CROWDPLATFORM;
	double dDisrep = 1;
	mReputations.erase(iWorker);

	printf("generally reputation:\n");
	for(map<int, double>::iterator it = mReputations.begin();
		it != mReputations.end();
		it++)
	{
		//consider reputation, workload and distance
		it->second = it->second + 
			mWorkloads[it->first] -
			(float)(min(max(it->first, g_iWorker) - min(it->first, g_iWorker), (int)(min(it->first, g_iWorker) + mReputations.size() - max(it->first, g_iWorker)))) / 20;

			printf("%d %lf\n", it->first, it->second);
	}

	while ((1 - dDisrep < dRequirement) && (mReputations.size() > 0))
	{
		map<int, double>::iterator it = mReputations.begin();
		double dMaxRep = it->second;
		iWorker = it->first;
		for(;
			it != mReputations.end();
			it++)
		{
			if(dMaxRep < it->second)
			{
				dMaxRep = it->second;
				iWorker = it->first;
			}
		}
		vWorker.push_back(iWorker);
		mReputations.erase(iWorker);
		dDisrep = dDisrep * (1 - dMaxRep);
	}
	if(1 - dDisrep < dRequirement) return false;
	return true;
}

bool negotiate(int iPK, double dRequirement)
{
	if(g_tasks.findTx(iPK))
	{
		printf("task %d already created\n", iPK);
		return true;
	}

	//transfer file
	string strDetail;
	string strFileName = to_string(iPK) + ".dat";
	ifstream fin(strFileName.c_str());
	if(!fin)
	{
		fin.close();
		return false;
	}
	fin >> strDetail;
	fin.close();

	vector<int> vWorker;
	if(dRequirement < 1)
	{
		if(!selectNodes(vWorker, dRequirement))
		{
			printf("no suitable workers\n");
			return false;
		}

		string strWorkers, strDataWorker;
		for(vector<int>::iterator it = vWorker.begin();
			it != vWorker.end();
			it++)
		{
			if(*it != g_iWorker)
				strDataWorker = strDataWorker + " " + to_string(*it);
			strWorkers = strWorkers + " " + to_string(*it);
		}

		if(!sendMsg(strDataWorker, "data", iPK, strDetail, false))
			return false;
//		if(!sendMsg(SELF, CROWDPLATFORM, "newTask", iPK, strWorkers, strWorkers, false))
//			return false;
//		if(!sendMsg(WORKER, CROWDPLATFORM, "newTask", iPK, strWorkers, strWorkers, false))
//			return false;
		if(!sendMsg(strWorkers, "newTask", iPK, strWorkers, false))
			return false;
	}
	else
	{
		if(!sendMsg(WORKER, CROWDPLATFORM, "data", iPK, strDetail, "All", false))
			return false;
		if(!sendMsg(WORKER, CROWDPLATFORM, "newTask", iPK, "All", "All", false))
			return false;
	}
	return true;
}
