#ifndef __NEGOTIATE__
#define __NEGOTIATE__

#include <vector>
#include <fstream>
#include <sstream>
#include "load.h"
#include "client.h"
#include "tx.h"

bool negotiate(int iPK, double dRequirement);
void printReputation();

extern int g_iWorker;
extern Tasks g_tasks;

#endif