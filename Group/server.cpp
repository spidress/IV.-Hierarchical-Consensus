#include "server.h"


int startServer()
{
#ifdef _WIN32
	system("del server.log");
#else
	system("rm server.log");
#endif
	string strIP;
	int iPort;
	bool bWork;

	string strWorkerDetail;
	if(!loadWorker(strWorkerDetail)) 
	{
		printf("load worker error");
		return -1;
	}
	stringstream ssWorkerDetail(strWorkerDetail);
	ssWorkerDetail >> g_iWorker >> strIP >> iPort >> bWork;
	//load worker
	if(g_iWorker < 0)
	{
		printf("loadWorker error");
		return false;
	}

	char szCmd[4096];
#ifdef _WIN32
	SOCKET listenfd, connfd;
	SOCKADDR_IN serveraddr;
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,0),&wsa);    //初始化WS2_32.DLL
#else
	int    listenfd, connfd;
	struct sockaddr_in     serveraddr;
#endif
	char    buff[4096];
	int     len;

	//创建套接字
#ifdef _WIN32
	if((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0) 
	{
		printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
		return -1;
	}
#else
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
		return -1;
	}
#endif

	printf("openning IP = %s, Port = %d\n", strIP.c_str(), iPort);
	memset(&serveraddr, 0, sizeof(serveraddr));

	//命名协议，IP，端口
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(iPort);
#ifdef _WIN32
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#else
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif

	//绑定套接字
#ifdef _WIN32
	printf("Win32\n");
	if(::bind(listenfd, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) != 0)
	{
		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
		return -1;
	}
#else
	printf("Linux\n");
	if(bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1){
		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
		return -1;
	}
#endif

	if( listen(listenfd, 10) == -1){
		printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
		return -1;
	}

	//监听请求
	printf("======waiting for client's request======\n");
	while(1)
	{
#ifdef _WIN32
		//if((connfd = accept(listenfd, (SOCKADDR *)&clientaddr, &len))<=0)
		if((connfd = accept(listenfd, (SOCKADDR *)NULL, 0))<=0)
		{
			printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
			continue;
		}
//		len = recv(connfd, buff, MAXLINE, 0);
//		if(len <= 0)    
//		{
//			printf("close connection!\n");
//			closesocket(connfd);
//			continue;
//		}

#else
		if((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1)
		{
			printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
			close(connfd);
			continue;
		}
//		len = recv(connfd, buff, MAXLINE, 0);
//		if(len <= 0)
//		{
//			printf("close connection!\n");
//			close(connfd);
//			continue;
//		}
#endif
		int iRetryCnt = 20;
		int iRetry = 0;
		do
		{
			len = recv(connfd, buff, MAXLINE, 0);
			//printf("retry to recv iRetry=%d len=%d\n", iRetry,len);
			iRetry++;
		} while ((len <= 0) && (iRetry < iRetryCnt));//retry 5 times
		if(iRetry >= iRetryCnt)
			continue;

		buff[len] = '\0';
		printf("recv msg from client: %s at %0.lf\n", buff, loadTime());
		sprintf(szCmd, "echo %s >> server.log", buff);
		system(szCmd);

		stringstream ssBuff(buff);
		string strAction;
		int pk;
		int iFrom;

		//process the messages to server
		ssBuff >> strAction >> pk >> iFrom;
		string strMsg, strTmp;
		int iCnt = 0;
		while(ssBuff >> strTmp)
		{
			if(iCnt == 0)
				strMsg = strTmp;
			else
				strMsg = strMsg + " " + strTmp;
			iCnt++;
		}
//		if((strAction != "genTask") &&
//			(strAction != "print") && 
//			(strAction != "data"))
//		{
//			sendMsg(EXCL, iFrom, strAction, pk, strMsg, false);
//		}
		string strInfoMsg = strAction + " " + to_string(pk) + " " + strMsg;
		//avoid duplicated msg
		if(g_setMsg.find(strInfoMsg) != g_setMsg.end())
			continue;
		if((strAction != "print") && (strAction != "stop") && (strAction != "query") && (strAction != "reputation"))  
			g_setMsg.insert(strInfoMsg);
		
		//write to main.log
		ofstream fout("main.log", ios::app|ios::out);
		if(!fout)
		{
			fout.close();
			printf("open main.log err\n");
#ifdef _WIN32
			closesocket(connfd);
#else
			close(connfd);
#endif
			break;
		}
		fout << buff << " " << loadTime() << endl;
		fout.close();

		if(strAction == "data")
		{
			string strFile = to_string(pk) + ".dat";
//			ifstream fFile(strFile) ;
//			if(fFile.is_open())
//			{
//				fFile.close();
//				continue;
//			}

			stringstream ssCommand;
			ssCommand << "echo " << strTmp << " > " << strFile;
			system(ssCommand.str().c_str());
		}
		else if(strAction == "stop")
		{
			printf("stop to exit\n");
			//alert other nodes
			if(!sendMsg(WORKER, iFrom, strAction, 0, "", "", false))
				return false;
#ifdef _WIN32
			Sleep(SLEEPTIME);
#else
			usleep(SLEEPTIME*1000);
#endif
			system("echo 1 > stop.txt");
#ifdef _WIN32
			closesocket(connfd);
#else
			close(connfd);
#endif
			break;
		}
		//genTask then negotiate
		else if(strAction == "genTask")
		{
			//if requirement is specified
			if(strMsg == "")
			{
				if(!negotiate(pk, 1))
					continue;
			}
			else
			{
				double dRequirement = atof(strMsg.c_str());
				if(!negotiate(pk, dRequirement))
					continue;
			}
		}

		//newTask then add to chain
		else if(strAction == "newTask")
		{
			thread tNewJob(newJob, pk, iFrom, strMsg);
			tNewJob.detach();
		}
		else if(strAction == "reputation")
		{
			printReputation();
		}
		//ask query
		else if(strAction == "query")
		{
			int iTo = atoi(strTmp.c_str());
			if(iTo != g_iWorker)
			{
				printf("iTo %d is not current worker %d\n", iTo, g_iWorker);
				continue;
			}
			thread tQueryJob(queryJob, pk, iFrom);
			tQueryJob.detach();
		}

		//receive Solution then run chain
		else if(strAction == "recSolution")
		{
			if(!g_RecStack.is_full())
				g_RecStack.write(ssBuff.str());
			thread tRecJob(recJob, true);
			tRecJob.detach();
		}

		else if(strAction == "recBlock")
		{
			thread tRecBlock(recBlock, pk, iFrom, strMsg);
			tRecBlock.detach();
		}

		else if(strAction == "disBlock")
		{
			thread tRecBlock(disBlock, pk, iFrom, strMsg);
			tRecBlock.detach();
		}


		else if(strAction == "print")
		{
			thread tPrintJob(printJob);
			tPrintJob.detach();
		}

#ifdef _WIN32
		closesocket(connfd);
#else
		close(connfd);
#endif
	}
#ifdef _WIN32
	WSACleanup();     //释放WS2_32.DLL
#else
	close(listenfd);
#endif
	return 0;
}
