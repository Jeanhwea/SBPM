#ifndef  _DATA_H_
#define  _DATA_H_

#include <string>
#include "tinyxml/tinyxml.h"

using namespace std;

extern int sz_task;
extern int sz_resource;
extern float * array_duration;
extern bool * matrix_depend;
extern bool * matrix_assign;

int loadXML(string full_filename);
int allocMemory();
int freeMemory();

#endif //!_DATA_H_