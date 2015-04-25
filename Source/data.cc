#include "data.h"

size_t sz_task;
size_t sz_resource;

float * array_duration;
float * array_resource_occupy;
float * matrix_reso_alloc;
bool  * matrix_depend;
bool  * matrix_assign;

using namespace std;

TiXmlDocument   * pxmldoc;
TiXmlElement    * proot,
                * psize,
                * ptasks,
                * presources,
                * pdependencies,
                * passignments;

int initElements(string full_filename);
int loadInfo();


int loadXML(string full_filename) 
{
    initElements(full_filename);
    dataAllocMemory();
    loadInfo();
    return 0;
}


int initElements(string full_filename)
{
    pxmldoc = new TiXmlDocument(full_filename);
    int ret = pxmldoc->LoadFile();
    assert(ret != 0);
    proot = pxmldoc->RootElement();
    assert(proot != 0);
    psize = proot->FirstChildElement("Size");
    assert(psize != 0);
    ptasks = proot->FirstChildElement("Tasks");
    assert(ptasks != 0);
    presources = proot->FirstChildElement("Resources");
    assert(presources != 0);
    pdependencies = proot->FirstChildElement("Dependencies");
    assert(pdependencies != 0);
    passignments = proot->FirstChildElement("Assignments");
    assert(passignments != 0);

    return 0;
}

int dataAllocMemory() 
{
    string str_size;
    str_size = psize->Attribute("TaskSize");
    sz_task = stoi(str_size);
    str_size = psize->Attribute("ResourceSize");
    sz_resource = stoi(str_size);

    array_duration = (float *) calloc(sz_task, sizeof(float));
    if (array_duration == 0) {
        fprintf(stderr, "Error: cannot alloc memory!!!");
        assert(0);
    }

    array_resource_occupy = (float *) calloc(sz_resource, sizeof(float));
    if (array_resource_occupy == 0) {
        fprintf(stderr, "Error: cannot alloc memory!!!");
        assert(0);
    }

    matrix_reso_alloc = (float *) calloc(sz_task * sz_resource, sizeof(float));
    if (matrix_reso_alloc == 0) {
        fprintf(stderr, "Error: cannot alloc memory!!!");
        assert(0);
    }

    matrix_depend = (bool *) calloc(sz_task * sz_task, sizeof(bool));
    if (matrix_depend == 0) {
        fprintf(stderr, "Error: cannot alloc memory!!!");
        assert(0);
    }

    matrix_assign = (bool *) calloc(sz_task * sz_resource, sizeof(bool));
    if (matrix_assign == 0) {
        fprintf(stderr, "Error: cannot alloc memory!!!");
        assert(0);
    }

    return 0;
}

int dataFreeMemory()
{
    if (array_duration != 0) {
        free(array_duration);
        array_duration = 0;
    }
    if (array_resource_occupy != 0) {
        free(array_resource_occupy);
        array_resource_occupy = 0;
    }
    if (matrix_reso_alloc != 0) {
        free(matrix_reso_alloc);
        matrix_reso_alloc = 0;
    }
    if (matrix_depend != 0) {
        free(matrix_depend);
        matrix_depend = 0;
    }
    if (matrix_assign != 0) {
        free(matrix_assign);
        matrix_assign = 0;
    }
    return 0;
}


int loadInfo()
{
    string str_text;
    TiXmlElement * pele;

    for (pele = ptasks->FirstChildElement("Task"); pele != 0; pele = pele->NextSiblingElement("Task")) {
        int task_id;
        float duration;
        str_text = pele->Attribute("id");
        task_id = stoi(str_text);
        str_text = pele->Attribute("duration");
        duration = stof(str_text);
        array_duration[task_id-1] = duration;
    }

    for (pele = pdependencies->FirstChildElement("Dependency"); pele != 0; pele = pele->NextSiblingElement("Dependency")) {
        int pred_id, succ_id;
        str_text = pele->Attribute("predecessor");
        pred_id = stoi(str_text);
        str_text = pele->Attribute("successor");
        succ_id = stoi(str_text);
        matrix_depend[(pred_id-1) + (succ_id-1) * sz_task] = true;
    }

    for (pele = passignments->FirstChildElement("Assignment"); pele != 0; pele = pele->NextSiblingElement("Assignment")) {
        int task_id, reso_id;
        str_text = pele->Attribute("task");
        task_id = stoi(str_text);
        str_text = pele->Attribute("resource");
        reso_id = stoi(str_text);
        matrix_assign[(task_id-1) + (reso_id-1) * sz_task] = true;
    }

    return 0;
}

/************************************************************************/
/* return true if <succ> depends <pred>                                 */
/************************************************************************/
bool isDepend(size_t pred_id, size_t succ_id)
{
    assert(pred_id <= sz_task);
    assert(succ_id <= sz_task);
    return matrix_depend[(pred_id-1) + (succ_id-1) * sz_task];
}

/************************************************************************/
/*  return true if <task> needs <reso>                                  */
/************************************************************************/
bool isAssign(size_t task_id, size_t reso_id)
{
    assert(task_id <= sz_task);
    assert(reso_id <= sz_resource);
    return matrix_assign[(task_id-1) + (reso_id-1) * sz_task];
}

/************************************************************************/
/* return duration of <task>                                            */
/************************************************************************/
float getDuration(size_t task_id)
{
    assert(task_id <= sz_task);
    return array_duration[task_id-1];
}


// ---- resource management ----
void clearResouceOccupy()
{
    size_t i, j;

    for (i = 0; i < sz_resource; i++) {
        array_resource_occupy[i] = 0.0f;
    }

    for (i = 0; i < sz_resource; i++) {
        for (j = 0; j < sz_task; j++) {
            matrix_reso_alloc[j + i * sz_task] = 0.0f;
        }
    }
}

float allocResouce(size_t task_id, size_t resource_id, float duration)
{
    assert(task_id <= sz_task);
    assert(resource_id <= sz_resource);
    matrix_reso_alloc[(task_id-1) + (resource_id-1) * sz_task] = duration;
    array_resource_occupy[resource_id-1] += duration;
    return array_resource_occupy[resource_id-1];
}

/************************************************************************/
/* find a resource with maximum occupancy and return it                 */
/************************************************************************/
float getOverheadDuration()
{
    size_t i, resource_id;
    float max;
    
    resource_id = 1;
    max = array_resource_occupy[0];
    for (i = 0; i < sz_resource; i++) {
        if (array_resource_occupy[i] > max) {
            resource_id = i+1;
            max = array_resource_occupy[i];
        }
    }

    return max;
}

float getOccupancy(size_t resource_id)
{
    assert(resource_id <= sz_resource);
    return array_resource_occupy[resource_id-1];
}

void dbPrintInfo()
{
    size_t i, j;
    printf("task duration\n");
    for (i = 1; i <= sz_task; i++) {
        printf("T%d=%f\n", i, getDuration(i));
    }
    printf("resources assignment\n");
    for (i = 1; i <= sz_resource; i++) {
        for (j = 1; j <= sz_task; j++) {
            printf("%d\t", isAssign(j, i));
        }
        printf("\n");
    }
    printf("resources occupy tabel\n");
    for (i = 1; i <= sz_resource; i++) {
        for (j = 1; j <= sz_task; j++) {
            printf("%.1f\t", matrix_reso_alloc[(j-1)+(i-1)*sz_task]);
        }
        printf("\n");
    }
}

