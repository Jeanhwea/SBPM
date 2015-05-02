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
int * matrix_chromo_slted;
/************************************************************************/
/* hash value for each person                                           */
/************************************************************************/
unsigned long * array_hashval;
unsigned long * array_hashval_slted;
/************************************************************************/
/* you can get fitness value like this:                                 */
/*      array_fitvalue[ task_id-1 ]];                                   */
/************************************************************************/
float * array_fitvalue;
float * array_fitvalue_slted;
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
void gaStatistics(FILE * out);
static float gaObject(int * person); // object function

// private tool functions
static bool swapBits(size_t a, size_t b, int * person);
static bool check(int * person);
static void personCopy(int * des, int * src, size_t begin, size_t n);
static int * personNew(int * old_person, size_t n);
static void personDestroy(int * person);
static void personClear(int * person, size_t n);
static void personMoveForward(int * person, size_t ele_index, size_t step);

static void calcAllFitvalue();
static int fitvalueCompare(const void *a, const void *b);
static void fixPerson(int * person);
void scheFCFS(int * person);

// debug functions
static void dbPrintPerson(int * person, size_t n, char * tag);
static void dbPrintFitvalue();
void dbDisplayWorld();

void dbDisplayWorld()
{
    size_t i;
    for (i = 0; i < ga_popsize; i++) {;
        char tag[100];
        sprintf(tag, "i%d\th%d\tf%f\t",i, array_hashval[i], array_fitvalue[i]);
        dbPrintPerson(matrix_chromo+i*sz_task, sz_task, tag);
    }

    for (i = 0; i < ga_popsize; i++) {
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
    for (i = 0; i < ga_popsize*2; i++) {
        printf("%d\t%d\t%f\n", i, array_fitval_order[i], array_fitvalue[array_fitval_order[i]-1]);
    }
}


void gaEvolve()
{
    size_t i, j, k, n;
    int * person, * new_person;
    FILE * fd_info;
    fd_info = fopen("output.txt", "w");
    assert(fd_info != 0);


    gaInitPara();
    gaAllocMemory();

    // for (i = 0; i < ga_popsize; i++) {
    //     person = matrix_chromo + i * sz_task;
    //     gaInit(person);
    //     array_hashval[i] = hashfunc(person, sz_task);
    //     array_fitvalue[i] = gaObject(person);
    // }   
    i = 0; 
    while (i < ga_popsize) {
        new_person = matrix_chromo + i * sz_task;
        gaInit(new_person);
        array_hashval[i] = hashfunc(new_person, sz_task);

        bool skip_flag = true;
        for (j = 0; j < i; j++) {
            // check for this individual
            if (array_hashval[i] == array_hashval[j]) {
                person = matrix_chromo + j*sz_task;
                for (k = 0; k < sz_task; k++) {
                    if (new_person[k] != person[k])
                        break;
                }
                if (k == sz_task) {
                    // need re-initialization
                    skip_flag = false;
                    break;
                }
            }
        }
        if (skip_flag) {
            // skip this individual if passed check
            array_fitvalue[i] = gaObject(new_person);
            i++;
        } else {
            fprintf(stderr, "re-initialization\n");
        }
    }
    printf("\n--------------- initial -------------------\n");
    dbDisplayWorld();

    for (n = 0; n < ga_ngen; n++) {
        printf("\n--------------- %3d -------------------\n", n);
        // for (i = 0; i < ga_popsize; i++) {
        //     array_hashval[i] = hashfunc(matrix_chromo+i*sz_task, sz_task);
        // }
        // for (i = 0; i < ga_popsize/2; i++) {
        //     int * dad, * mom, * bro, * sis;
        //     int a, b;
        //     a = genRandInt(0, ga_popsize-1);
        //     b = genRandInt(0, ga_popsize-1);
        //     dad = matrix_chromo + a*sz_task;
        //     mom = matrix_chromo + b*sz_task;
        //     bro = matrix_chromo + (2*i+ga_popsize)*sz_task;
        //     sis = matrix_chromo + (2*i+1+ga_popsize)*sz_task;
        //     gaCrossover(dad, mom, bro, sis);
        //     fixPerson(bro);
        //     fixPerson(sis);
        // }

        i = ga_popsize;
        while (i < 2*ga_popsize-1) {
            int * dad, * mom, * bro, * sis;
            int a, b;

            // randomly choose a dad and a mom
            a = genRandInt(0, ga_popsize-1);
            b = genRandInt(0, ga_popsize-1);
            dad = matrix_chromo + a*sz_task;
            mom = matrix_chromo + b*sz_task;

            // get bro and sis 's pointer
            bro = matrix_chromo + i*sz_task;
            sis = matrix_chromo + (i+1)*sz_task;
            gaCrossover(dad, mom, bro, sis);

            // fix each individual for their violation of constraint
            if (!check(bro)) {
                fixPerson(bro);
            }
            if (!check(sis)) {
                fixPerson(sis);
            }
            array_hashval[i]   = hashfunc(bro, sz_task);
            array_hashval[i+1] = hashfunc(sis, sz_task);

            bool skip_flag = true;
            for (j = 0; j < i; j++) {
                // check for brother
                if (array_hashval[i] == array_hashval[j]) {
                    person = matrix_chromo + j*sz_task;
                    for (k = 0; k < sz_task; k++) {
                        if (bro[k] != person[k])
                            break;
                    }
                    if (k == sz_task) {
                        // need re-crossover
                        skip_flag = false;
                        break;
                    }
                }
                // check for sister
                if (array_hashval[i+1] == array_hashval[j]) {
                    person = matrix_chromo + j*sz_task;
                    for (k = 0; k < sz_task; k++) {
                        if (sis[k] != person[k])
                            break;
                    }
                    if (k == sz_task) {
                        // need re-crossover
                        skip_flag = false;
                        break;
                    }
                }
            }
            if (skip_flag) {
                // skip this bro & sis if passed check
                i += 2;
            } else {
                fprintf(stderr, "re-crossover\n");
            }
        }

        for (i = 0; i < ga_popsize*2; i++) {
            gaMutation(matrix_chromo+i*sz_task);
        }

        // calculate attribution of children
        for (i = ga_popsize; i < ga_popsize*2; i++) {
            person = matrix_chromo+i*sz_task;
            array_fitvalue[i] = gaObject(person);
            array_hashval[i] = hashfunc(person, sz_task);
        }
        // dbPrintInfo();

        gaSelection();
        gaStatistics(fd_info);
        dbDisplayWorld();

    } // end of this generation


    // dbPrint(matrix_chromo, sz_task, "chromosome(001)");
    // dbPrint(matrix_chromo+sz_task, sz_task, "chromosome(002)");
    // dbPrint(matrix_chromo+sz_task*ga_popsize, sz_task, "chromosome(009)");
    // dbPrint(matrix_chromo+sz_task*(ga_popsize+1), sz_task, "chromosome(010)");

    fclose(fd_info);
    gaFreeMemory();
}

void gaInitPara()
{
    ga_popsize        = 80;
    ga_ngen           = 100;
    ga_prob_crossover = 0.8f;
    ga_prob_mutation  = 0.005f;
}

/************************************************************************/
/* Initialize a person                                                  */
/************************************************************************/
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
        // swap(person[a], person[b]);
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
        // swap(person[a], person[b]);
    }

}

/************************************************************************/
/* calculate fitness value, and move the bests to the parent of next    */
/*     generation.                                                      */
/************************************************************************/
void gaSelection()
{
    size_t i;

    // initial ordering first
    for (i = 0; i < ga_popsize*2; i++) {
        array_fitval_order[i] = i+1;
    }
    qsort(array_fitval_order, 2*ga_popsize, sizeof(size_t), fitvalueCompare);
    // dbPrintFitvalue();


    size_t selected_id;
    for (i = 0; i < ga_popsize; i++) {
        selected_id = array_fitval_order[i]-1;
        personCopy(matrix_chromo_slted+i*sz_task, matrix_chromo+(selected_id)*sz_task, 0, sz_task);
        array_fitvalue_slted[i] = array_fitvalue[selected_id];
        array_hashval_slted[i] = array_hashval[selected_id];
    }
    // move the selected value to next generation
    memcpy(matrix_chromo, matrix_chromo_slted, ga_popsize*sz_task*sizeof(int));
    memcpy(array_fitvalue, array_fitvalue_slted, ga_popsize*sizeof(float));
    memcpy(array_hashval, array_hashval_slted, ga_popsize*sizeof(unsigned long));
}

/************************************************************************/
/* Statistics of some important information.                            */
/************************************************************************/
void gaStatistics(FILE * out)
{
    size_t i;

    for (i = 0; i < ga_popsize; i++) {
        fprintf(out, "%f%c", array_fitvalue[i], i==ga_popsize-1 ? '\n': ' ');
    }
}

int gaAllocMemory()
{

    // chromosome attribution of a person
    matrix_chromo = (int *) calloc(sz_task * ga_popsize * 2, sizeof(int));
    assert(matrix_chromo != 0);
    matrix_chromo_slted = (int *) calloc(sz_task * ga_popsize, sizeof(int));
    assert(matrix_chromo_slted != 0);

    // hash value attribution of a person
    array_hashval = (unsigned long *) calloc(ga_popsize * 2, sizeof(unsigned long));
    assert(array_hashval != 0);
    array_hashval_slted = (unsigned long *) calloc(ga_popsize, sizeof(unsigned long));
    assert(array_hashval_slted != 0);

    // fitness value attribution of a person
    array_fitvalue = (float *) calloc(ga_popsize * 2, sizeof(float));
    assert(array_fitvalue != 0);
    array_fitvalue_slted = (float *) calloc(ga_popsize, sizeof(float));
    assert(array_fitvalue_slted != 0);

    // for selection ordering
    array_fitval_order = (size_t *)calloc(ga_popsize * 2, sizeof(size_t));
    assert(array_fitval_order != 0);
    for (size_t i = 0; i < ga_popsize*2; i++) {
        array_fitval_order[i] = i+1;
    }
    
    return 0;
}

int gaFreeMemory()
{
    if (matrix_chromo != 0) {
        free(matrix_chromo);
        matrix_chromo = 0;
    }
    if (matrix_chromo_slted != 0) {
        free(matrix_chromo_slted);
        matrix_chromo_slted = 0;
    }

    if (array_hashval != 0) {
        free(array_hashval);
        array_hashval = 0;
    }
    if (array_hashval_slted != 0) {
        free(array_hashval_slted);
        array_hashval_slted = 0;
    }
    
    if (array_fitvalue != 0) {
        free(array_fitvalue);
        array_fitvalue = 0;
    }
    if (array_fitvalue_slted != 0) {
        free(array_fitvalue_slted);
        array_fitvalue_slted = 0;
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
/* move a person[ele_index] several steps forward                       */
/************************************************************************/
static void personMoveForward(int * person, size_t ele_index, size_t step)
{
    int tmp;
    size_t i;
    assert(ele_index < sz_task);
    assert(ele_index + step < sz_task);
    tmp = person[ele_index];
    for (i = ele_index; i < ele_index + step; i++) {
        person[i] = person[i+1];
    }
    person[ele_index+step] = tmp;
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
    if (check(person)) {
        scheFCFS(person);
        score = getOverheadDuration();
        if (score == 0.0f) {
            score = INF_DURATION;
            fprintf(stderr, "Error, for zero score\n");
        }
    } else {
        fprintf(stderr, "Error, for a person is not pass check\n");
        score = INF_DURATION;
    }
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
                array_fitvalue[i] = INF_DURATION;
                fprintf(stderr, "Error, for zero score\n");
            } else {
                array_fitvalue[i] = score;
            }
        } else {
            // set 0.0f, if not pass checker
            score = INF_DURATION;
            array_fitvalue[i] = INF_DURATION;
        }
        sum += array_fitvalue[i];
        // printf("%f\n", score);
    }

    // normalize the fitness value
    // for (i = 0; i < ga_popsize*2; i++) {
    //     array_fitvalue[i] = array_fitvalue[i] / sum;
    // }
}

static int fitvalueCompare(const void *a, const void *b)
{
    return (array_fitvalue[(*(size_t *)a)-1] > array_fitvalue[(*(size_t *)b)-1]) ? 1: -1;
}

static void fixPerson(int * person)
{
    size_t i, j, step;
    i = 0;
    while (i < sz_task) {                       // FOR all tasks listed in person array

        // Number of steps to move elements forward?
        step = 0;
        for (j = i+1; j < sz_task; j++) {
            if (isDepend(person[j], person[i]))
                step = j-i;
        }

        if (step > 0) {
            personMoveForward(person, i, step);
        } else {
            // if no use to move, then i++
            i++;
        }

    }
}
