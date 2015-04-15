#include "ga.h"
#include "helper.h"
#include "data.h"

int ga_popsize;
int ga_ngen;
float ga_pc;
float ga_pm;

// chromosome for [sz_taks * ga_popsize*2]
int * matrix_chromo;
// each fitvalue for echo chromosome [ga_popsize*2]
float * array_fitvalue;

using namespace std;
void gaInitPara();
void gaInitPop();
void gaDisplayWorld();

// private functions
inline int idx2TaskID(int idx, int chromo_id);
static bool swapBits(int a, int b, int chromo_id);

void gaEvolve()
{
    gaInitPara();
    gaAllocMemory();

    gaInitPop();
    gaDisplayWorld();

    gaFreeMemory();
}

void gaInitPara()
{
    ga_popsize  = 8;
    ga_ngen     = 100;
    ga_pc       = 0.8f;
    ga_pm       = 0.005f;
}

void gaInitPop()
{
    int i, j;
    for (i = 0; i < ga_popsize; i++) {
        for (j = 0; j <sz_task; j++) {
            matrix_chromo[j + i * sz_task] = j+1;
        }
    }

    int a, b;
    for (i = 0; i < ga_popsize; i++) {
        for (j = 0; j <sz_task; j++) {
            // pick a random int to swap
            a = genRandInt(0, sz_task-1);
            b = j;
            if (a > b) {
                swap(a, b);
            }
            swapBits(a, b, i);
        }
    }
}

void gaDisplayWorld()
{
    int i, j;
    for (i = 0; i < ga_popsize*2; i++) {
        printf("chromosome(%03d): ", i+1);
        for (j = 0; j < sz_task; j++) {
            printf("%d", matrix_chromo[j + i * sz_task]);
            if (j< sz_task-1) {
                printf("->");
            } else {
                printf("\n");
            }
        }
    }
}

int gaAllocMemory()
{

    matrix_chromo = (int *) calloc(sz_task * ga_popsize * 2, sizeof(int));
    assert(matrix_chromo != 0);

    array_fitvalue = (float *) calloc(ga_popsize * 2, sizeof(float));
    assert(array_fitvalue != 0);

    return 0;
}

int gaFreeMemory()
{
    if (matrix_chromo != 0) {
        free(matrix_chromo);
        matrix_chromo = 0;
    }
    if (array_fitvalue != 0) {
        free(array_fitvalue);
        array_fitvalue = 0;
    }
    return 0;
}

// return true, if a-th task swap with b-th task; otherwise, return false.
static bool swapBits(int a, int b, int chromo_id)
{
    bool ret = true;
    // notice that, a < b
    if (a >= b) {
        ret = false;
    } else {
        int a_task_id, b_task_id, k_task_id;
        a_task_id = idx2TaskID(a, chromo_id);
        b_task_id = idx2TaskID(b, chromo_id);
        for (int k = a; k <= b; k++) {
            k_task_id = idx2TaskID(k, chromo_id);
            if ( (k!=a) && isDepend(a_task_id, k_task_id) ){
                ret = false;
                break;
            }
            if ( (k!=b) && isDepend(k_task_id, b_task_id) ) {
                ret = false;
                break;
            }
        }
    }
    
    if (ret) {
        int tmp;
        tmp = matrix_chromo[a + chromo_id * sz_task];
        matrix_chromo[a + chromo_id * sz_task] = matrix_chromo[b + chromo_id * sz_task];
        matrix_chromo[b + chromo_id * sz_task] = tmp;
    }

    return ret;
}

inline int idx2TaskID(int idx, int chromo_id)
{
    return matrix_chromo[idx + chromo_id * sz_task];
}

bool check(int id)
{
    bool ret = false;

    return ret;
}