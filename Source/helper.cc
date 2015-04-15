#include "helper.h"

bool isSrand = false;

int genRandInt(int min, int max)
{
    if (isSrand == false) {
        unsigned int seed = time(0);
        srand(seed);
        isSrand = true;
    }
    int randInt = rand() % (max-min+1);
    return min + randInt;
}

float genRandProb()
{    
    if (isSrand == false) {
        unsigned int seed = time(0);
        srand(seed);
        isSrand = true;
    }
    int randInt = rand() % 100000;
    return (float)(randInt) / 100000.0f;
}