#include "helper.h"

int genRandInt(int min, int max)
{
    int randInt = rand() % (max-min);
    return min + randInt;
}