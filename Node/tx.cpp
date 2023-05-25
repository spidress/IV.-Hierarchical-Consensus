#include "tx.h"

int section = 20;
int interval = 60;

void Tx::print()
{
	printf("TP_public_key = %d\n", TP_public_key);
	printf("Height = %d\n", Height);
	printf("WorkerCnt = %d\n", WorkerCnt);
	printf("PreWeight = %d\n", PreWeight);
	printf("Worker = %d\n", Worker);
	printf("E_solution = %d\n", E_solution);
	printf("process_time = %.0lf\n", process_time);
	printf("preWorker:\n");
	for(set<Tx*>::iterator it = preWorkers.begin(); it != preWorkers.end(); it++)
		printf("%d\n", (*it)->getWorker());
	printf("sucWorker:\n");
	for(set<Tx*>::iterator it = sucWorkers.begin(); it != sucWorkers.end(); it++)
		printf("%d\n", (*it)->getWorker());
	printf("preTask = %d\n", preTask?preTask->getTP_public_key():-1);
	printf("subTask = %d\n", sucTask?sucTask->getTP_public_key():-1);
	printf("\n");
}

void Tx::writeFile(string strFileName)
{
	FILE *fout;
	if((fout = fopen(strFileName.c_str(), "a+")) == NULL)
	{
		printf("write file %s error\n", strFileName.c_str());
		return;
	}
	fprintf(fout, "TP_public_key = %d\n", TP_public_key);
	fprintf(fout, "Height = %d\n", Height);
	fprintf(fout, "WorkerCnt = %d\n", WorkerCnt);
	fprintf(fout, "PreWeight = %d\n", PreWeight);
	fprintf(fout, "Worker = %d\n", Worker);
	fprintf(fout, "E_solution = %d\n", E_solution);
	fprintf(fout, "process_time = %.0lf\n", process_time);
	fprintf(fout, "preWorker:\n");
	for(set<Tx*>::iterator it = preWorkers.begin(); it != preWorkers.end(); it++)
		fprintf(fout, "%d\n", (*it)->getWorker());
	fprintf(fout, "sucWorker:\n");
	for(set<Tx*>::iterator it = sucWorkers.begin(); it != sucWorkers.end(); it++)
		fprintf(fout, "%d\n", (*it)->getWorker());
	fprintf(fout, "preTask = %d\n", preTask?preTask->getTP_public_key():0);
	fprintf(fout, "subTask = %d\n", sucTask?sucTask->getTP_public_key():0);
	fprintf(fout, "\n");
	fclose(fout);
}

Tx* Tasks::findTx(int t_TP_public_key, int iWorker)
{
	if(iWorker == CROWDPLATFORM)
	{
		if(TaskGenesises.find(t_TP_public_key) == TaskGenesises.end())
			return NULL;
		else
			return TaskGenesises[t_TP_public_key];
	}
	bool bFound = false;
	if(WorkerTips.find(iWorker) == WorkerTips.end())
		return NULL;
	Tx* tx = WorkerTips[iWorker];
	while(tx)
	{
		if(tx->getTP_public_key() == t_TP_public_key)
		{
			bFound = true;
			return tx;
		}
		tx = tx->getpreTask();
	}
	return NULL;
}

bool Tasks::newTask(int t_TP_public_key, string strWorkers)
{
	for(map<int, Tx*>::iterator it = TaskGenesises.begin() ;it!=TaskGenesises.end();it++)
	{
		if(it->first == t_TP_public_key)
		{
			printf("newTask pk = %d already exists\n", t_TP_public_key);
			return false;
		}
	}
	Tx *t = new Tx(t_TP_public_key, strWorkers);
	TaskGenesises.insert(map<int, Tx*>::value_type(t_TP_public_key, t));
	Tips.insert(t);
	allTx.insert(t);
	allWorkers[CROWDPLATFORM].push_back(t);
//		t->print();
	return true;
}

bool Tasks::newTask(int t_TP_public_key, int workerCnt)
{
	for(map<int, Tx*>::iterator it = TaskGenesises.begin() ;it!=TaskGenesises.end();it++)
	{
		if(it->first == t_TP_public_key)
		{
			printf("newTask pk = %d already exists\n", t_TP_public_key);
			return false;
		}
	}
	Tx *t = new Tx(t_TP_public_key, workerCnt);
	TaskGenesises.insert(map<int, Tx*>::value_type(t_TP_public_key, t));
	Tips.insert(t);
	allTx.insert(t);
//		t->print();
	return true;
}

bool Tasks::findWorker(set<Tx*> t, int t_TP_public_key, int t_Worker)
{
	for(set<Tx*>::iterator it=t.begin() ;it != t.end();it++)
	{
		if((t_Worker == (*it)->getWorker()) &&
			t_TP_public_key == (*it)->getTP_public_key() &&
			(*it)->getSolution() != 0)
		{
			return true;
		}
		findWorker((*it)->getpreWorkers(), t_TP_public_key, t_Worker);
	}
	return false;
}

bool Tasks::findTx(set<Tx*> t_txs, int t_TP_public_key, int t_Solution, int t_Height, set<int> t_preIntWorkers, set<Tx*> &t_preWorkers)
{
	for(set<Tx*>::iterator it=t_txs.begin() ;it != t_txs.end();it++)
	{
		//			(*it)->print();
		//found related chain
		if (t_TP_public_key == (*it)->getTP_public_key() &&
			/*t_Height <= (*it)->getHeight() &&*/
			t_preIntWorkers.find((*it)->getWorker()) != t_preIntWorkers.end())
		{
			if(t_Solution == (*it)->getSolution())
				t_preWorkers.insert(*it);
		}
		//notfound, then ask for it
	}
	if(t_preWorkers.size() == t_preIntWorkers.size())
		return true;
	else
	{
		for(set<int>::iterator it = t_preIntWorkers.begin();
			it != t_preIntWorkers.end();
			it++)
		{
			bool bFound = false;
			for(set<Tx*>::iterator it1 = t_preWorkers.begin();
				it1 != t_preWorkers.end();
				it1++)
			{
				if((*it1)->getWorker() == *it)
				{
					bFound = true;
					break;
				}
			}
//			if(!bFound)
//			{
//				if(!g_NotFound.is_full())
//				{
//					string strNotFound = "query " + to_string(t_TP_public_key) + " " + to_string(g_iWorker) + " " + to_string(*it);
//					g_NotFound.write(strNotFound);
//				}
//			}
		}
		return false;
	}
}

bool Tasks::setWorkerCnt(Tx* &t)
{
	set<Tx*> preWorker = t->getpreWorkers();
	for(set<Tx*>::iterator it = preWorker.begin(); it != preWorker.end(); it++)
	{
		if(t->getWorkerCnt())
		{
			if(t->getWorkerCnt() != (*it)->getWorkerCnt())
			{
				return false;
			}
		}
		else
		{
			t->setWorkerCnt((*it)->getWorkerCnt());
		}
	}
	return true;
}

string Tasks::getBlock(int iPK)
{
	Tx* tx = TaskGenesises[iPK];
	set<string> setBlkMsg;
	string strBlkMsg = "";
	getBlkMsg(tx, setBlkMsg);
	for(set<string>::iterator it = setBlkMsg.begin();
		it != setBlkMsg.end();
		it++)
		strBlkMsg += *it + " -2 ";
	return strBlkMsg;
}

bool Tasks::getBlkMsg(Tx* tx, set<string> &setBlkMsg)
{
	if(tx->getHeight() > 0)
	{
		string strBlkMsg = getTxMsg(tx);
		if(setBlkMsg.find(strBlkMsg) == setBlkMsg.end())
			setBlkMsg.insert(strBlkMsg);
	}

	set<Tx*> sSucWorkers = tx->getsucWorkers();
	for(set<Tx*>::iterator it = sSucWorkers.begin();
		it != sSucWorkers.end();
		it++)
	{
		getBlkMsg(*it, setBlkMsg);
	}
	return true;
}

string Tasks::getTxMsg(Tx* tx)
{
	string strBlkMsg;
	if(tx) 
	{
		strBlkMsg += to_string(tx->getWorker()) + " " + 
			     to_string(tx->getSolution()) + " " +
				 to_string(tx->getHeight()) + " " ;
		set<Tx*> sPreWorkers = tx->getpreWorkers();
		
		for(set<Tx*>::iterator it = sPreWorkers.begin();
			it != sPreWorkers.end();
			it++)
		{
			strBlkMsg += to_string((*it)->getWorker()) + " ";
		}
	}
	return strBlkMsg;
}

int getWeight(Tx* t)
{
	int iWeight = 1;
	set<Tx*> sucWorkers = t->getsucWorkers();
	for(set<Tx*>::iterator it = sucWorkers.begin();
		it != sucWorkers.end();
		it++)
		iWeight += getWeight(*it);
	return iWeight;
}

bool Tasks::isFinished(int iPK, int iWorkerCnt)
{
	int iMaxWeight = 0;
	int iSecWeight = 0;
	int iTotalWeight = 0;
	if(TaskGenesises.find(iPK) == TaskGenesises.end())
	{
		if(!newTask(iPK, CROWDPLATFORM))
			return false;
	}
	set<Tx*> Txs = TaskGenesises[iPK]->getsucWorkers();
	for(set<Tx*>::iterator it = Txs.begin(); 
		it != Txs.end();
		it++)
	{
		int iWeight = getWeight(*it);
		if(iMaxWeight <= iWeight)
		{
			iSecWeight = iMaxWeight;
			iMaxWeight = iWeight;
		}
		iTotalWeight += iWeight;
	}
	int iNull = iWorkerCnt - iTotalWeight;
	if(iSecWeight + iNull <= iMaxWeight)
		return true;
	else
		return false;
}

string Tasks::addTx(int t_TP_public_key, 
					int t_Worker,
					int t_Solution,
					int &t_height,
					set<int> &t_preIntWorkers,
					string &strFriends)
{
	string strMessage2Servers = "";
	Tx* t = new Tx(t_TP_public_key, t_Worker, t_Solution);
	set<Tx*> preWorkers;
	//find tips with same solution and pk
	for(set<Tx*>::iterator it=Tips.begin() ;it!=Tips.end();it++)
	{
		if(((*it)->getTP_public_key() == t_TP_public_key) &&
			((*it)->getSolution() == t_Solution))
		{
			preWorkers.insert(*it);
		}
	}
	//find tips with same pk
	if(preWorkers.size() == 0)
	{
		for(map<int, Tx*>::iterator it=TaskGenesises.begin() ;it!=TaskGenesises.end();it++)
		{
			if(it->first == t_TP_public_key)
			{
				preWorkers.insert(it->second);
				break;
			}
		}
	}
	if(preWorkers.size() == 0)
	{
		printf("cannot find preWorkers\n");
		return "";
	}

	//avoid duplicated tx
	if(findWorker(Tips, t->getTP_public_key(), t->getWorker()))
	{
		printf("addTx TP_public_key=%d, worker = %d already exists\n", t->getTP_public_key(), t->getWorker());
		return "";
	}
	int maxHeight = 0;
	for(set<Tx*>::iterator it = preWorkers.begin(); it != preWorkers.end(); it++)
	{
		if(maxHeight < (*it)->getHeight())
			maxHeight = (*it)->getHeight();
		t->addpreWorkers(*it);     //set preWorkers as preWorkers for t
		//			t->print();
		(*it)->addsucWorkers(t);   //set sucWorkers = t for preWorkers
		Tips.erase(*it);    //remove preWorkers from Tips;
	}
	t->setHeight(maxHeight + 1);
	Tips.insert(t);

	//setWorkerCnt
	if(setWorkerCnt(t) == false) return "";

	//addWorkerTips
	if(WorkerTips.find(t->getWorker()) != WorkerTips.end())
	{
		t->setpreTask(WorkerTips[t->getWorker()]);
		Tx* tmp = WorkerTips[t->getWorker()];
		tmp->setsucTask(t);
	}
	WorkerTips[t->getWorker()] = t;

	allTx.insert(t);
	allWorkers[t_Worker].push_back(t);
	//feedback height & preWorker
	t_height = t->getHeight();
	for(set<Tx*>::iterator it = preWorkers.begin(); it != preWorkers.end(); it++)
	{
		t_preIntWorkers.insert((*it)->getWorker());
	}

	//setPreWeight
	set<Tx*> allPreWorker; //all workers that are pre of t
	getAllPreWorker(t->getpreWorkers(), allPreWorker);
	t->setPreWeight(allPreWorker.size());

	//record LongTips
	if(LongTips.find(t->getTP_public_key()) != LongTips.end())
	{
		Tx* tmp = LongTips[t->getTP_public_key()];
		if(t->getPreWeight() > tmp->getPreWeight())
		{
			LongTips[t->getTP_public_key()] = t;
		}
	}
	else
	{
		LongTips.insert(map<int, Tx*>::value_type(t->getTP_public_key(), t));
	}

	//judge t whether dominant
	map<int, int> mFriends;
	int tWeight = 0;
	if(TaskGenesises[t->getTP_public_key()]->getRemarks() == "All")
	{
		for(map<int, string>::iterator it = g_mWorkerDetails.begin();
			it != g_mWorkerDetails.end();
			it++)
		{
			if(it->first != g_iWorker)
				mFriends[it->first]++;
		}
	}
	else
	{
		stringstream ssFriends(TaskGenesises[t->getTP_public_key()]->getRemarks());
		int iFriend;
		while(ssFriends >> iFriend) mFriends[iFriend]++;
	}
	for(set<Tx*>::iterator it = Tips.begin();
		it != Tips.end();
		it++)
	{
		if((*it)->getTP_public_key() == t->getTP_public_key()) 
		{
			if(((*it)->getWorker() != t->getWorker()) &&
				(tWeight < (*it)->getPreWeight()))
				tWeight = (*it)->getPreWeight();
		}
		Tx* tTx = *it;
		while(tTx)
		{
			if(mFriends[tTx->getWorker()] == 0) break;
			else mFriends[tTx->getWorker()]--;
		}
	}

	int iBusy = 0;
	for(map<int, int>::iterator itB = mFriends.begin();
		itB != mFriends.end();
		itB++)
	{
		if(itB->second > 0) iBusy++;
	}

//	//if solution confirmed, send message to Nofriend
//	if(t->getPreWeight() >= tWeight + iBusy)
//	{
//		bDone = true;
//	}

	//message to all clients
	stringstream ssendLine;
	ssendLine //<< "recSolution "
		      //<< t_TP_public_key << " " 
			  //<< t_Worker << " " //iFrom
			  << t_Worker << " " //iWorker
			  << t_Solution << " " 
			  << t_height << " ";
	for(set<Tx*>::iterator it = preWorkers.begin(); it != preWorkers.end(); it++)
	//for(set<Tx*>::iterator it = allPreWorker.begin(); it != allPreWorker.end(); it++)
	{
		ssendLine << (*it)->getWorker() << " ";
	}
	strMessage2Servers = ssendLine.str();
	strFriends = TaskGenesises[t_TP_public_key]->getRemarks();
 	return strMessage2Servers;
}


bool Tasks::recTx(int t_TP_public_key, int t_Worker, int t_Solution, int t_height, set<int> t_preIntWorkers)
{
	//ensure preWorkers exist
	set<Tx*> preWorkers;
	Tx* t;
	if(t_height == 1)
	{
		if(TaskGenesises.find(t_TP_public_key) == TaskGenesises.end())
		{
			t = new Tx(t_TP_public_key, t_Worker, t_Solution, t_height);
			TaskGenesises.insert(make_pair(t_TP_public_key,t));
		}
		else
		{
			//point genesis
			set<Tx*> t_sucWorkers = TaskGenesises[t_TP_public_key]->getsucWorkers();
			for(set<Tx*>::iterator it1 = t_sucWorkers.begin();
				it1 != t_sucWorkers.end();
				it1++)
			{
				if ((t_TP_public_key == (*it1)->getTP_public_key()) && 
					(t_Worker == (*it1)->getWorker()))
				{
					return true;
				}
			}
			t = new Tx(t_TP_public_key, t_Worker, t_Solution, t_height);
			t->addpreWorkers(TaskGenesises[t_TP_public_key]);
			TaskGenesises[t_TP_public_key]->addsucWorkers(t);
			Tips.erase(TaskGenesises[t_TP_public_key]);
			Tips.insert(t);

			if(setWorkerCnt(t) == false) return false;

			//setPreWeight
			set<Tx*> allPreWorker;
			getAllPreWorker(t->getpreWorkers(), allPreWorker);
			t->setPreWeight(allPreWorker.size());

			//avoid duplicated tx
			if(findWorker(allTx, t->getTP_public_key(), t->getWorker()))
			{
				printf("recTx TP_public_key=%d, worker = %d already exists\n", t->getTP_public_key(), t->getWorker());
				return true;
			}

			//record LongTips
			if(LongTips.find(t->getTP_public_key()) != LongTips.end())
			{
				Tx* tmp = LongTips[t->getTP_public_key()];
				if(t->getPreWeight() > tmp->getPreWeight())
				{
					LongTips[t->getTP_public_key()] = t;
				}
			}
			else
			{
				LongTips.insert(map<int, Tx*>::value_type(t->getTP_public_key(), t));
			}

			//addWorkerTips
			if(WorkerTips.find(t->getWorker()) != WorkerTips.end())
			{
				t->setpreTask(WorkerTips[t->getWorker()]);
				Tx* tmp = WorkerTips[t->getWorker()];
				tmp->setsucTask(t);
			}
			WorkerTips[t->getWorker()] = t;

			allTx.insert(t);
			allWorkers[t_Worker].push_back(t);
			return true;
		}		
	}
	else if(!findTx(allTx, t_TP_public_key, t_Solution, t_height-1, t_preIntWorkers, preWorkers)) 
	{
		printf("findTx err: TP_public_key = %d,  Solution = %d, Height = %d, preIntWorkers: ", t_TP_public_key, t_Solution, t_height-1);
		for(set<int>::iterator it = t_preIntWorkers.begin(); it != t_preIntWorkers.end(); it++)
		{
			printf("%d ", *it);
		}
		printf("\n");

		return false;
	}

	//avoid duplicated tx
	//if(findWorker(Tips, t_TP_public_key, t_Worker))
	if(findTx(t_TP_public_key, t_Worker) != NULL)
	{
		printf("findWorker err: recTx TP_public_key=%d, worker = %d already exists\n", t_TP_public_key, t_Worker);
		return true;
	}

	//check whether height is correct
	int maxHeight = 0;
	for(set<Tx*>::iterator it = preWorkers.begin(); it != preWorkers.end(); it++)
	{
		if(maxHeight < (*it)->getHeight())
			maxHeight = (*it)->getHeight();
	}
	if(maxHeight != t_height - 1)
	{
		printf("err: recTx TP_public_key=%d, worker = %d, solution = %d, height = %d error\n",
			t_TP_public_key, t_Worker, t_Solution, t_height);
//		if(!g_NotFound.is_full())
//		{
//			string strNotFound = "query " + to_string(t_TP_public_key) + " " + to_string(t_Worker) + " " + to_string(g_iWorker);
//			g_NotFound.write(strNotFound);
//		}
		return true;
	}

	//create Tx
	t = new Tx(t_TP_public_key, t_Worker, t_Solution, t_height);

	//link to chain
	for(set<Tx*>::iterator it = preWorkers.begin(); it != preWorkers.end(); it++)
	{
		t->addpreWorkers(*it);     //set preWorkers as preWorkers for t
		(*it)->addsucWorkers(t);   //set sucWorkers = t for preWorkers
		Tips.erase(*it);    //remove preWorkers from Tips;
	}

	//setPreWeight
	set<Tx*> allPreWorker;
	getAllPreWorker(t->getpreWorkers(), allPreWorker);
	t->setPreWeight(allPreWorker.size());

	//record LongTips
	if(LongTips.find(t->getTP_public_key()) != LongTips.end())
	{
		Tx* tmp = LongTips[t->getTP_public_key()];
		if(t->getPreWeight() > tmp->getPreWeight())
		{
			LongTips[t->getTP_public_key()] = t;
		}
//		Tx* tmp = TaskGenesises[t->getTP_public_key()];
//		tmp->Remarks
	}
	else
	{
		LongTips.insert(map<int, Tx*>::value_type(t->getTP_public_key(), t));
	}

	Tips.insert(t);

	if(setWorkerCnt(t) == false) 
	{
		printf("setWorkerCnt err:");
		return true;
	}

	//addWorkerTips
	if(WorkerTips.find(t->getWorker()) != WorkerTips.end())
	{
		t->setpreTask(WorkerTips[t->getWorker()]);
		Tx* tmp = WorkerTips[t->getWorker()];
		tmp->setsucTask(t);
	}
	WorkerTips[t->getWorker()] = t;

	allTx.insert(t);
	allWorkers[t_Worker].push_back(t);
	return true;
}

void calReputation(map<int, double> &mReputations, double per, int TipHeight, double distance, set<Tx*> txs)
{
	for(set<Tx*>::iterator it = txs.begin();
		it != txs.end();
		it++)
	{
		if((*it)->getWorker() == CROWDPLATFORM) break;
		mReputations[(*it)->getWorker()] +=  per * (TipHeight - (*it)->getHeight()) / (double)TipHeight * distance / 2; // to release the impact
		if((*it)->getpreWorkers().size() > 0)
		{
			calReputation(mReputations, 1, TipHeight, distance, (*it)->getpreWorkers());
		}
	}
}

bool calWorkload(set<int> &sWorkers, set<Tx*> txs)
{
	for(set<Tx*>::iterator it = txs.begin();
		it != txs.end();
		it++)
	{
		sWorkers.erase((*it)->getWorker());
		calWorkload(sWorkers, (*it)->getsucWorkers());
	}
	return true;
}

bool Tasks::getReputations(map<int, double> &mReputations)
{
	double dNow = loadTime();
	int iTipCnt = 0;
	if(!loadWorkers(g_mWorkerDetails)) return false;
	for(map<int, string>::iterator it = g_mWorkerDetails.begin();
		it != g_mWorkerDetails.end();
		it++)
	{
		mReputations[it->first] = INIT_CREDIBILITY;
	}
	for(map<int, Tx*>::iterator it = LongTips.begin();
		it != LongTips.end();
		it++)
	{
		double distance = 1 - (dNow - TaskGenesises[it->first]->getTime()) / (interval * section);
		if(distance <= 0) continue;
		int TipHeight = it->second->getHeight();
		//mReputations[it->second->getWorker()] += (TipHeight -  TipHeight) / TipHeight * distance;
		calReputation(mReputations, 1-INIT_CREDIBILITY, TipHeight, distance, it->second->getpreWorkers());
	}
	return true;
}

bool Tasks::getWorkloads(map<int, double> &mWorkloads)
{
	double dNow = loadTime();

	for(map<int, Tx*>::iterator it = TaskGenesises.begin();
		it != TaskGenesises.end();
		it++)
	{
		double distance = 1 - (dNow - TaskGenesises[it->first]->getTime()) / (interval * section);
		if(distance <= 0) continue;
		stringstream ssWorkers(it->second->getRemarks());
		set<int> sWorkers;
		int iWorker;
		while(ssWorkers >> iWorker)
			sWorkers.insert(iWorker);
		sWorkers.erase(CROWDPLATFORM);
		calWorkload(sWorkers, it->second->getsucWorkers());
		for(set<int>::iterator it = sWorkers.begin();
			it != sWorkers.end();
			it++)
			mWorkloads[*it] = mWorkloads[*it] - distance / 2; // to release the impact
	}
	return true;
}