/**
 * File: helper.h
 * Author: Jeanhwea
 * Email: hujinghui@buaa.edu.cn
 */

#ifndef _HELPER_H_
#define _HELPER_H_
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int genRandInt(int min, int max);
float genRandProb();
unsigned long hashfunc(int * person, size_t num);

#endif // !_HELPER_H_
