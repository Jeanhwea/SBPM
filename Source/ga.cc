#include "ga.h"
#include "helper.h"
#include "data.h"
#include <string.h>
#include <stdio.h>

size_t ga_popsize;
size_t ga_ngen;
float  ga_prob_crossover;
float  ga_prob_mutation;

// chromosome for [sz_taks * ga_popsize*2]
int * matrix_chromo;


/************************************************************************/
/* you can get fitness value like this:                                 */
/*      array_fitvalue[ task_id-1 ]];                    */
/************************************************************************/
float * array_fitvalue;
/************************************************************************/
/* you can get ordering of task_id like this:                           */
/*      array_fitval_order[ task_id-1 ];                                */
/************************************************************************/
size_t * array_fitval_order;

using namespace std;
void gaInitPara();
void gaInit(int * person);
void gaCrossover(int * dad, int * mom, int * bro, int * sis);
void gaMutation(int * person);
void gaSelection();
static float gaObject(int * person); // object function

// private tool functions
static bool swapBits(size_t a, size_t b, int * person);
static bool check(int * person);
static void personCopy(int * des, int * src, size_t begin, size_t n);
static int * personNew(int * old_person, size_t n);
static void personDestroy(int * person);
static void personClear(int * person, size_t n);
static void calcAllFitvalue();
static int fitvalueCompare(const void *a, const void *b);

void scheFCFS(int * person);

// debug functions
static void dbPrintPerson(int * person, size_t n, char * tag);
static void dbPrintFitvalue();
void dbDisplayWorld();

void gaEvolve()
{
    size_t i, n;


    gaInitPara();
    gaAllocMemory();

    for (i = 0; i < ga_popsize; i++) {
        gaInit(matrix_chromo + i * sz_task);
    }
    dbDisplayWorld();

    for (n = 99; n < ga_ngen; n++) {
        printf("\n--------------- %3d -------------------\n", n);
        for (i = 0; i < ga_popsize / 2; i++) {
            int * dad, * mom, * bro, * sis;
            dad = matrix_chromo + 2*i*sz_task;
            mom = matrix_chromo + (2*i+1)*sz_task;
            bro = dad + ga_popsize*sz_task;
            sis = mom + ga_popsize*sz_task;
            gaCrossover(dad, mom, bro, sis);
        }
        dbDisplayWorld();

        // dbPrintInfo();
        gaSelection();
    }


    // dbPrint(matrix_chromo, sz_task, "chromosome(001)");
    // dbPrint(matrix_chromo+sz_task, sz_task, "chromosome(002)");
    // dbPrint(matrix_chromo+sz_task*ga_popsize, sz_task, "chromosome(009)");
    // dbPrint(matrix_chromo+sz_task*(ga_popsize+1), sz_task, "chromosome(010)");

    gaFreeMemory();
}

void gaInitPara()
{
    ga_popsize        = 8;
    ga_ngen           = 100;
    ga_prob_crossover = 0.8f;
    ga_prob_mutation  = 0.005f;
}


void dbDisplayWorld()
{
    size_t i;
    for (i = 0; i < ga_popsize*2; i++) {
        char tag[100];
        sprintf(tag, "chromo(%03d)", i+1);
        dbPrintPerson(matrix_chromo+i*sz_task, sz_task, tag);
    }

    for (i = 0; i < ga_popsize*2; i++) {
        if (!check(matrix_chromo + i*sz_task)) {
            fprintf(stderr, "chromo(%3d) failed in check\n", i+1);
        }
    }
}

static void dbPrintPerson(int * person, size_t n, char * tag)
{
    size_t i;

    printf("%s : ", tag);
    for (i = 0; i < n; i++) {
        printf("%d", person[i]);
        if (i < n-1) {
            printf("->");
        } else {
            printf("\n");
        }
    }

}

static void dbPrintFitvalue()
{
    size_t i;
    printf("index\ttask_id\tfitval\n");
    for (i = 0; i < ga_popsize*2; i++)
        printf("%d\t%d\t%f\n", i, array_fitval_order[i], array_fitvalue[array_fitval_order[i]-1]);
}

void gaInit(int * person)
{
    size_t i;
    for (i = 0; i < sz_task; i++) {
        person[i] = i+1;
    }

    size_t a, b;
    for (i = 0; i < sz_task; i++) {
        a = genRandInt(0, sz_task-1);
        b = i; 
        if (a > b) {
            swap(a, b);
        }

        swapBits(a, b, person);
    }
}

/************************************************************************/
/* ordering-based two points crossover                                  */
/************************************************************************/
void gaCrossover(int * dad, int * mom, int * bro, int * sis)
{
    size_t i, j, k, a, b;
    int * dad_new, * mom_new;
    if (genRandProb() < ga_prob_crossover) {
        a = genRandInt(0, sz_task-1);
        b = genRandInt(0, sz_task-1);
        if (a > b) {
            swap(a, b);
        }

        dad_new = personNew(dad, sz_task);
        mom_new = personNew(mom, sz_task);
        personClear(bro, sz_task);
        personClear(sis, sz_task);

        // copy selected region first
        personCopy(bro, mom, a, b-a+1);
        personCopy(sis, dad, a, b-a+1);

        // remove duplicated items
        for (k = 0; k < sz_task; k++) {
            for (i = a; i <= b; i++) {
                if (dad_new[k] == mom[i]) {
                    dad_new[k] = 0;
                    break;
                }
            }
            for (i = a; i <= b; i++) {
                if (mom_new[k] == dad[i]) {
                    mom_new[k] = 0;
                    break;
                }
            }
        }
        
        // dbPrint(bro, sz_task, "bro-con");
        // dbPrint(sis, sz_task, "sis-con");
        // dbPrint(dad_new, sz_task, "bro-ord");
        // dbPrint(mom_new, sz_task, "sis-ord");
        
        // copy remainder region
        i = j = 0;
        for (k = 0; k < sz_task; k++) {
            if (bro[k] == 0) {
                for (; i < sz_task; i++) {
                    if (dad_new[i] != 0) {
                        bro[k] = dad_new[i++];
                        break;
                    }
                }
            }
            if (sis[k] == 0) {
                for (; j < sz_task; j++) {
                    if (mom_new[j] != 0) {
                        sis[k] = mom_new[j++];
                        break;
                    }
                }
            }
        }

        // printf("crosspoint %d %d\n", a, b);
        // dbPrint(dad, sz_task, "dad    ");
        // dbPrint(mom, sz_task, "mom    ");
        // dbPrint(bro, sz_task, "bro    ");
        // dbPrint(sis, sz_task, "sis    ");
        personDestroy(dad_new);
        personDestroy(mom_new);

    } else {
        personCopy(bro, dad, 0, sz_task);
        personCopy(sis, mom, 0, sz_task);
    }
}

/************************************************************************/
/* two points swap mutation                                             */
/************************************************************************/
void gaMutation(int * person)
{
    size_t a, b;

    if (genRandProb() < ga_prob_mutation) {
        a = genRandInt(0, sz_task-1);
        b = genRandInt(0, sz_task-1);
        if (a > b) {
            swap(a, b);
        }

        swapBits(a, b, person);
    }

}

void gaSelection()
{
    size_t i, j;
    calcAllFitvalue();
    dbPrintFitvalue();
    qsort(array_fitval_order, 2*ga_popsize, sizeof(size_t), fitvalueCompare);
    dbPrintFitvalue();
}

int gaAllocMemory()
{

    matrix_chromo = (int *) calloc(sz_task * ga_popsize * 2, sizeof(int));
    assert(matrix_chromo != 0);

    array_fitvalue = (float *) calloc(ga_popsize * 2, sizeof(float));
    assert(array_fitvalue != 0);

    array_fitval_order = (size_t *)calloc(ga_popsize * 2, sizeof(size_t));
    assert(array_fitval_order != 0);

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

    if (array_fitval_order != 0) {
        free(array_fitval_order);
        array_fitval_order = 0;
    }

    return 0;
}

/****************************************************************************/
/* return true, if a-th task swap with b-th task; otherwise, return false.  */
/****************************************************************************/
static bool swapBits(size_t a, size_t b, int * person)
{
    bool ret = true;
    // notice that, a < b
    if (a >= b) {
        ret = false;
    } else {
        size_t i, a_task_id, b_task_id, k_task_id;
        a_task_id = person[a];
        b_task_id = person[b];
        for (i = a; i <= b; i++) {
            k_task_id = person[i];
            if ( (i!=a) && isDepend(a_task_id, k_task_id) ){
                ret = false;
                break;
            }
            if ( (i!=b) && isDepend(k_task_id, b_task_id) ) {
                ret = false;
                break;
            }
        }
    }
    
    if (ret) {
        swap(person[a], person[b]);
    }

    return ret;
}


static void personCopy(int * des, int * src, size_t begin, size_t n)
{
    size_t i;
    for (i = begin; i < begin + n; i++) {
        des[i] = src[i];
    }
}

static int * personNew(int * old_person, size_t n)
{
    int * new_person;
    new_person = (int *) calloc(n, sizeof(int));
    personCopy(new_person, old_person, 0, n);
    return new_person;
}

static void personDestroy(int * person)
{
    free(person);
}

static void personClear(int * person, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        person[i] = 0;
    }
}

/************************************************************************/
/*   feasibility check for <chromo_id>-th chromosome.                   */
/*       return true, if pass; otherwise, return false                  */
/************************************************************************/
static bool check(int * person)
{
    size_t i, j;
    for (i = 0; i < sz_task; i++) {
        for (j = i+1; j < sz_task; j++) {
            int i_task_id, j_task_id;
            i_task_id = person[i];
            j_task_id = person[j];

            if (isDepend(j_task_id, i_task_id)) {
                // printf("failed depend %d -> %d\n", j_task_id, i_task_id);
                return false;
            }
        }
    }
    return true;
}

static float gaObject(int * person)
{
    float score;
    scheFCFS(person);
    score = getOverheadDuration();
    return score;
}

/************************************************************************/
/* scheduler, implement FCFS (first come, first service).               */
/************************************************************************/
void scheFCFS(int * person)
{
    size_t i, r, task_id;

    // set temporary data struct as 0
    clearResouceOccupy();
    for (i = 0; i < sz_task; i++) {
        task_id = person[i];
        float dura = getDuration(task_id);

        size_t min_id = 0;
        float min_occ, occ;
        for (r = 1; r <= sz_resource; r++) { // search all resources
            if (isAssign(task_id,r)) {
                if (min_id == 0) {
                    min_occ = getOccupancy(r);
                    min_id = r;
                } else {
                    occ = getOccupancy(r);
                    if (occ < min_occ) {
                        min_occ = occ;
                        min_id = r;
                    }
                }
            }
        }

        if (min_id > 0) {
            allocResouce(task_id, min_id, dura);
        } else {
            allocResouce(task_id, 1, dura);
        }
    }
}

static void calcAllFitvalue()
{
    size_t i;
    float score, sum;
    // build chromosome id to fitness value index map first
    for (i = 0; i < ga_popsize*2; i++) {
        array_fitval_order[i] = i+1;
    }
    
    // calculate all fitness value (2*ga_popsize)
    sum = 0.0f;
    for (i = 0; i < ga_popsize*2; i++) {
        if (check(matrix_chromo+i*sz_task)) {
            score = gaObject(matrix_chromo+i*sz_task);
            if (score == 0.0f) {
                fprintf(stderr, "Error, for zero score\n");
            } else {
                array_fitvalue[i] = 1.0f / score;
            }
        } else {
            // set 0.0f, if not pass checker
            array_fitvalue[i] = 0.0f;
        }
        sum += array_fitvalue[i];
        printf("%f\n", array_fitvalue[i]);
    }

    // normalize the fitness value
    for (i = 0; i < ga_popsize*2; i++) {
        array_fitvalue[i] = array_fitvalue[i] / sum;
    }
}

static int fitvalueCompare(const void *a, const void *b)
{
    return (array_fitvalue[(*(size_t *)a)-1] < array_fitvalue[(*(size_t *)b)-1]) ? 1: -1;
}


