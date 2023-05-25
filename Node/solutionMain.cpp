#include "solutionMain.h"
#define SLEEPTIME 2

int main()
{
#ifdef _WIN32
	system("del stop.txt");
	system("del main.log");
	system("del client.log");
	system("del *.dat");
#else
	system("rm stop.txt");
	system("rm main.log");
	system("rm client.log");
	system("rm *.dat");
#endif
	g_iWorker = loadWorker();
	//load worker
	if(g_iWorker < 0)
	{
		printf("loadWorker error");
		return -1;
	}

	thread tServer(startServer);
	tServer.detach();

	//check to stop
	bool bstop = false;
	while(!bstop)
	{
#ifdef _WIN32
		Sleep(SLEEPTIME);
#else
		usleep(SLEEPTIME*1000);
#endif
		ifstream fstop;
		//check to stop
		fstop.open("stop.txt");
		if(fstop)
		{
			fstop >> bstop;
		}
		fstop.close();
	}

#ifdef _DEBUG
	getchar();
#endif
	return 0;
}