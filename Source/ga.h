#ifndef _GA_H_
#define _GA_H_
#include <string>

using namespace std;

extern int ga_popsize;
extern int ga_ngen;
extern int ga_pc;
extern int ga_pm;

int gaAllocMemory();
int gaFreeMemory();

#endif // !_GA_H_