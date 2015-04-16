#include "helper.h"

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