#ifndef _GA_H_
#define _GA_H_
#include <string>

using namespace std;

extern size_t ga_popsize;
extern size_t ga_ngen;

int gaAllocMemory();
int gaFreeMemory();
void gaEvolve();

#endif // !_GA_H_