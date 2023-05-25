#include "client.h"

int startClient()
{
	string strIP;
	int iPort;
	string strTmp;
	string strDetail;
	while (!g_SolutionStack.is_empty())
	{
		g_num_mutex.lock();
		if(!g_SolutionStack.read(&strTmp))
		{
			g_num_mutex.unlock();
			continue;
		}
		g_num_mutex.unlock();

		if(strTmp.length() == 0)
			continue;
		stringstream ss(strTmp);
		ss >> strIP >> iPort >> strDetail;
		string strTmpDetail;
		while(ss >> strTmpDetail)
		{
			strDetail = strDetail + " " + strTmpDetail;
		}
		double process_time = loadTime();

		//创建套接字
#ifdef _WIN32
		SOCKET sockfd;
		SOCKADDR_IN /*sockaddr_in,*/ servaddr;
		WSADATA wsa;
		if(WSAStartup(MAKEWORD(2,0),&wsa))	//初始化WS2_32.DLL
		{
			continue;
		}
#else
		int    sockfd;
		struct sockaddr_in    servaddr;
#endif
#ifdef _WIN32
		if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)
		{
			printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
			continue;
		}
#else
		if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
			continue;
		}
#endif
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(iPort);

#ifdef _WIN32
		servaddr.sin_addr.S_un.S_addr = inet_addr(strIP.c_str());
		if(connect(sockfd, (SOCKADDR *)&servaddr, sizeof(servaddr)) != 0)
		{
			printf("sending \"%s\" to IP = %s port = %d connect error: %s(errno: %d)\n", strTmp.c_str(), strIP.c_str(), iPort, strerror(errno),errno);
			continue;
		}
#else
		if( inet_pton(AF_INET, strIP.c_str(), &servaddr.sin_addr) <= 0){
			printf("inet_pton error for %s %d\n",strIP.c_str(), iPort);
			continue;
		}
		if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		{
			printf("sending \"%s\" to IP = %s port = %d connect error: %s(errno: %d)\n", strTmp.c_str(), strIP.c_str(), iPort, strerror(errno),errno);
			continue;
		}
#endif

		//printf("send msg to server: \n");
		char strCmd[512];
		sprintf(strCmd, "echo sent msg %s at time %.0lf >> client.log\n", strTmp.c_str(), process_time);
		system(strCmd);

		//printf(strCmd);

		if(send(sockfd, strDetail.c_str(), strDetail.length() + 1, 0) <= 0)
		//if(send(sockfd, strDetail.c_str(), strDetail.length() + 1, 0) < 0)
		{
			printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
			continue;
		}
//		g_setMsg.insert(strDetail);
#ifdef _WIN32
		if(closesocket(sockfd))
		{
			continue;
		}
		if(WSACleanup())    //释放WS2_32.DLL
		{
			continue;
		}
#else
		close(sockfd);
#endif

		int iSleepTime = 10;
#ifdef _WIN32
			Sleep(iSleepTime);
#else
			usleep(iSleepTime*1000);
#endif
	}
	return 0;
}

bool sendMsg(string strIP, int iPort, string strAction, int iPK, string strDetail)
{
	string strMsg = strAction + " " + to_string(iPK) + " " + to_string(g_iWorker) + " " + strDetail; 
	string strCmd = "echo " + strIP + " " + to_string(iPort) + ": " +strMsg + " >> client.log";
	system(strCmd.c_str());

//	if(strDetail.length() == 0)
//		return false;

	string strTmp = strIP + " " + to_string(iPort) + " " + strMsg;

	g_num_mutex.lock();
	g_SolutionStack.write(strTmp);
	g_num_mutex.unlock();

	thread tClient(startClient);
	tClient.detach();
	return true;
}

bool getGroupAdd(string &strIP, int &iPort)
{
	int iNo;
	ifstream fWorker("Group.txt");
	if(!fWorker)
	{
		fWorker.close();
		printf("cannot open Group.txt\n");
		return false;
	}
	fWorker >> iNo >> strIP >> iPort;
	fWorker.close();
	return true;
}

bool getECAdd(string &strIP, int &iPort)
{
	int iNo;
	ifstream fWorker("Order.txt");
	if(!fWorker)
	{
		fWorker.close();
		printf("cannot open Order.txt\n");
		return false;
	}
	fWorker >> iNo >> strIP >> iPort;
	fWorker.close();
	return true;
}

bool sendMsg(ToWhom iWhom,
			 int iFrom,
			 string strAction,
			 int iPK,
			 string strDetail,
			 string strFriends,
			 bool bConsiderWork,
			 int iExclude)
{
	string strMsg = strAction + " " + to_string(iPK) + " " + to_string(g_iWorker) + " " + strDetail; 
	string strCmd = "echo ";
	switch(iWhom)    //SELF = 0, WORKER = 1, ORDER = 2, GP = 3
	{
	case SELF:
		{
		strCmd = strCmd + "SELF: ";
		break;
		}
	case WORKER:
		{
		strCmd = strCmd + "WORKER: ";
		break;
		}
	case ORDER:
		{
		strCmd = strCmd + "ORDER: ";
		break;
		}
	case GP:
		{
		strCmd = strCmd + "GP: ";
		break;
		}
	case GPs:
		{
		strCmd = strCmd + "GPs: ";
		break;
		}
	}
	strCmd = strCmd + strMsg + " >> client.log";
	system(strCmd.c_str());

	if(iWhom == WORKER)
	{
		if(!loadAllWorkers(g_mWorkerDetails)) return false;

		int iTo = -1, iTo1 = -1;
		if(g_mWorkerDetails.find(g_iWorker) == g_mWorkerDetails.end())
		{
			map<int, string>::iterator it = g_mWorkerDetails.begin();
			if(it->first != iExclude)
				iTo = it->first;
			else
			{
				it++;
				if(it != g_mWorkerDetails.end())
					iTo = it->first;
			}
			map<int, string>::reverse_iterator it1 = g_mWorkerDetails.rbegin();
			if(it1->first != iExclude)
				iTo1 = it1->first;
			else
			{
				it1++;
				if(it1 != g_mWorkerDetails.rend())
					iTo1 = it1->first;
			}
		}
		else
		{
			for(map<int, string>::iterator it = g_mWorkerDetails.begin();
				it != g_mWorkerDetails.end();
				it++)
			{
				if(g_iWorker == it->first)
				{
					it++;
					if(it != g_mWorkerDetails.end())
						iTo = it->first;
					break;
				}				
			}
			if(iTo == -1)
				iTo = g_mWorkerDetails.begin()->first;
			for(map<int, string>::reverse_iterator it1 = g_mWorkerDetails.rbegin();
				it1 != g_mWorkerDetails.rend();
				it1++)
			{
				if(g_iWorker == it1->first)
				{
					it1++;
					if(it1 != g_mWorkerDetails.rend())
						iTo1 = it1->first;
					break;
				}
			}
			if(iTo1 == -1)
				iTo1 = g_mWorkerDetails.rbegin()->first;
		}
		if((iTo != iExclude) && (iTo != -1) && (iTo != iFrom)) 
		{
			stringstream ssGPDetail(g_mWorkerDetails[iTo]);
			int iGP;
			string strIP;
			int iPort;
			ssGPDetail >> iGP >> strIP >> iPort;
			sendMsg(strIP, iPort, strAction, iPK, strDetail);
		}
		if((iTo != iTo1) && (iTo1 != iExclude) && (iTo1 != -1) && (iTo1 != iFrom))
		{
			stringstream ssGPDetail(g_mWorkerDetails[iTo1]);
			int iGP;
			string strIP;
			int iPort;
			ssGPDetail >> iGP >> strIP >> iPort;
			sendMsg(strIP, iPort, strAction, iPK, strDetail);
		}
	}
	if(iWhom == GPs)
	{
		if(!loadAllGPs(g_mGroups)) return false;

		int iTo = -1, iTo1 = -1;
		if(g_mGroups.find(g_iWorker) == g_mGroups.end())
		{
			map<int, string>::iterator it = g_mGroups.begin();
			if(it->first != iExclude)
				iTo = it->first;
			else
			{
				it++;
				if(it != g_mGroups.end())
					iTo = it->first;
			}
			map<int, string>::reverse_iterator it1 = g_mGroups.rbegin();
			if(it1->first != iExclude)
				iTo1 = it1->first;
			else
			{
				it1++;
				if(it1 != g_mGroups.rend())
					iTo1 = it1->first;
			}
		}
		else
		{
			for(map<int, string>::iterator it = g_mGroups.begin();
				it != g_mGroups.end();
				it++)
			{
				if(g_iWorker == it->first)
				{
					it++;
					if(it != g_mGroups.end())
						iTo = it->first;
					break;
				}				
			}
			if(iTo == -1)
				iTo = g_mGroups.begin()->first;
			for(map<int, string>::reverse_iterator it1 = g_mGroups.rbegin();
				it1 != g_mGroups.rend();
				it1++)
			{
				if(g_iWorker == it1->first)
				{
					it1++;
					if(it1 != g_mGroups.rend())
						iTo1 = it1->first;
					break;
				}
			}
			if(iTo1 == -1)
				iTo1 = g_mGroups.rbegin()->first;
		}
		if((iTo != iExclude) && (iTo != -1) && (iTo != iFrom)) 
		{
			stringstream ssGPDetail(g_mGroups[iTo]);
			int iGP;
			string strIP;
			int iPort;
			ssGPDetail >> iGP >> strIP >> iPort;
			sendMsg(strIP, iPort, strAction, iPK, strDetail);
		}
		if((iTo != iTo1) && (iTo1 != iExclude) && (iTo1 != -1) && (iTo1 != iFrom))
		{
			stringstream ssGPDetail(g_mGroups[iTo1]);
			int iGP;
			string strIP;
			int iPort;
			ssGPDetail >> iGP >> strIP >> iPort;
			sendMsg(strIP, iPort, strAction, iPK, strDetail);
		}
//		for(map<int, string>::iterator it = g_mGroups.begin();
//			it != g_mGroups.end();
//			it++)
//		{
//			if(it->first != iExclude)
//			{
//				string strGPDetail = it->second;
//				stringstream ssGPDetail(strGPDetail);
//				int iGP;
//				string strIP;
//				int iPort;
//				ssGPDetail >> iGP >> strIP >> iPort;
//				sendMsg(strIP, iPort, strAction, iPK, strDetail);
//			}
//		}
	}
	else if(iWhom == GP)
	{
		string strIP;
		int iPort;
		getGroupAdd(strIP, iPort);
		if(!sendMsg(strIP, iPort, strAction, iPK, strDetail))
			return false;
	}
	if(iWhom == SELF)
	{
		if(!sendMsg(to_string(g_iWorker), strAction, iPK, strDetail, bConsiderWork))
			return false;
	}
	if(iWhom == ORDER)
	{
		string strIP;
		int iPort;
		getECAdd(strIP, iPort);
		if(!sendMsg(strIP, iPort, strAction, iPK, strDetail))
			return false;
	}
	return true;
}



bool sendMsg(string strWhom, string strAction, int iPK, string strDetail, bool bConsiderWork)
{
	string strMsg = strAction + " " + to_string(iPK) + " " + to_string(g_iWorker) + " " + strDetail; 

	string strCmd = "echo " + strWhom + ": " + strMsg + " >> client.log";
	system(strCmd.c_str());

//	if(strDetail.length() == 0)
//		return false;

	stringstream ssWhom(strWhom);
	int iWorker;
	while(ssWhom >> iWorker)
	{
		string strIP;
		int iPort;
		bool bWork;
		if(!loadWorkerDetail(iWorker, strIP, iPort, bWork))
			continue;
		if(bConsiderWork && !bWork)
			continue;
		string strTmp = strIP + " " + to_string(iPort) + " " + strMsg;
//		g_num_mutex.lock();
		g_SolutionStack.write(strTmp);
//		g_num_mutex.unlock();
		thread tClient(startClient);
		tClient.detach();
	}
	return true;
}
