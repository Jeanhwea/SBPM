#ifndef  _DATA_H_
#define  _DATA_H_

#include <string>
#include "tinyxml/tinyxml.h"

using namespace std;

extern int sz_task;
extern int sz_resource;

int loadXML(string full_filename);
int dataAllocMemory();
int dataFreeMemory();


bool isDepend(int pred, int succ);
bool isAssign(int task, int reso);

#endif //!_DATA_H_