#include "helper.h"

static unsigned long nInversion(int * person, size_t num);


bool isSrand = false;

/************************************************************************/
/* generate a random Integer (min <= Int <= max                         */
/************************************************************************/
int genRandInt(int min, int max)
{
    if (isSrand == false) {
        time_t seed = time(0);
        srand((unsigned int)seed);
        isSrand = true;
    }
    int randInt = rand() % (max-min+1);
    return min + randInt;
}

/************************************************************************/
/* generate a float probability ( 0 ~ 1 )                               */
/************************************************************************/
float genRandProb()
{    
    if (isSrand == false) {
       time_t seed = time(0);
        srand((unsigned int)seed);
        isSrand = true;
    }
    int randInt = rand() % 100000;
    return (float)(randInt) / 100000.0f;
}

/************************************************************************/
/* find a inversion number of a sequence                                */
/************************************************************************/
static unsigned long nInversion(int * person, size_t num)
{
    unsigned long ret = 0;
    size_t i, j;
    for (i = 0; i < num; i++) {
        for (j = i+1; j < num; j++) {
            if (person[i] > person[j])
                ret++;
        }
    }
    return ret;
}

unsigned long hashfunc(int * person, size_t num)
{
    unsigned long ret = nInversion(person, num);
    // ret += person[0] * num*(num-1)/2;
    return ret;
}