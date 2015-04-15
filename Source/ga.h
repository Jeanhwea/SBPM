#ifndef _GA_H_
#define _GA_H_
#include <string>

using namespace std;

extern int ga_popsize;
extern int ga_ngen;
extern float ga_pc;
extern float ga_pm;

int gaAllocMemory();
int gaFreeMemory();
void gaEvolve();

#endif // !_GA_H_