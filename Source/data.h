#ifndef  _DATA_H_
#define  _DATA_H_

#include <string>
#include "tinyxml/tinyxml.h"

using namespace std;

extern size_t sz_task;
extern size_t sz_resource;

int loadXML(string full_filename);
int dataAllocMemory();
int dataFreeMemory();


bool isDepend(size_t pred, size_t succ);
bool isAssign(size_t task, size_t reso);
float getDuration(size_t task);

#endif //!_DATA_H_