#include "data.h"

size_t sz_task;
size_t sz_resource;

float * array_duration;
float * array_resource_usage;
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

    array_resource_usage = (float *) calloc(sz_resource, sizeof(float));
    if (array_resource_usage == 0) {
        fprintf(stderr, "Error: cannot alloc memory!!!");
        assert(0);
    }

    matrix_depend = (bool *) calloc(sz_task*sz_task, sizeof(bool));
    if (matrix_depend == 0) {
        fprintf(stderr, "Error: cannot alloc memory!!!");
        assert(0);
    }

    matrix_assign = (bool *) calloc(sz_task*sz_resource, sizeof(bool));
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
        int id;
        float duration;
        str_text = pele->Attribute("id");
        id = stoi(str_text);
        str_text = pele->Attribute("duration");
        duration = stof(str_text);
        array_duration[id-1] = duration;
    }

    for (pele = pdependencies->FirstChildElement("Dependency"); pele != 0; pele = pele->NextSiblingElement("Dependency")) {
        int pred, succ;
        str_text = pele->Attribute("predecessor");
        pred = stoi(str_text);
        str_text = pele->Attribute("successor");
        succ = stoi(str_text);
        matrix_depend[(pred-1) + (succ-1) * sz_task] = true;
    }

    for (pele = passignments->FirstChildElement("Assignment"); pele != 0; pele = pele->NextSiblingElement("Assignment")) {
        int task, reso;
        str_text = pele->Attribute("task");
        task = stoi(str_text);
        str_text = pele->Attribute("resource");
        reso = stoi(str_text);
        matrix_assign[(task-1) + (reso-1) * sz_task] = true;
    }

    return 0;
}

/************************************************************************/
/* return true if <succ> depends <pred>                                 */
/************************************************************************/
bool isDepend(size_t pred, size_t succ)
{
    return matrix_depend[(pred-1) + (succ-1) * sz_task];
}

/************************************************************************/
/*  return true if <task> needs <reso>                                  */
/************************************************************************/
bool isAssign(size_t task, size_t reso)
{
    return matrix_assign[(task-1) + (reso-1) * sz_task];
}

/************************************************************************/
/* return duration of <task>                                            */
/************************************************************************/
float getDuration(size_t task)
{
    return array_duration[task-1];
}


// ---- resource management ----
void clearResouceUsage()
{
    size_t i;
    for (i = 0; i < sz_resource; i++)
        array_resource_usage[i] = 0.0f;
}

float allocResouce(size_t resource_id, float duration)
{
    array_resource_usage[resource_id-1] += duration;
    return array_resource_usage[resource_id-1];
}

int findMinUsage()
{
    size_t i, ret;
    float min;
    
    ret = 0;
    min = 0.0f;
    for (i = 0; i < sz_resource; i++) {
        if (array_resource_usage[i] < min) {
            min = array_resource_usage[i];
            ret = i;
        }
    }

    // return resource_id (ret+1)
    return (ret+1);
}