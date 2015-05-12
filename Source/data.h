/**
 * File: data.h
 * Author: Jeanhwea
 * Email: hujinghui@buaa.edu.cn
 */

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


bool isDepend(size_t pred_id, size_t succ_id);
bool isAssign(size_t task_id, size_t reso_id);
float getDuration(size_t task_id);
void clearResouceOccupy();
float allocResouce(size_t task_id, size_t resource_id, float duration);
float getOccupancy(size_t resource_id);
float  getOverheadDuration();

// debug functions
void dbPrintInfo();

#endif //!_DATA_H_