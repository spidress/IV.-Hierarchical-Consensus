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
	
	Blk *bGenesis = new Blk();
	g_blk = bGenesis;
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

	Blk* tBlk;
	while(g_blk)
	{
		tBlk =  g_blk;
		g_blk = g_blk->preBlk;

		char szOutput[512];
		sprintf(szOutput, "echo %s > blocks.log\n", tBlk->strBlk.c_str());
		system(szOutput);
		delete(tBlk);
	}
#ifdef _DEBUG
	getchar();
#endif
	return 0;
}