#ifndef VARIABLES_H
#define VARIABLES_H

#include "../../general/log/log.h"

const size_t MAX_VARIABLES = 100;

struct Variable
{
    char *name;
    double val;
};

struct VariablesTable
{
    size_t size;
    Variable vars[MAX_VARIABLES];
};

Variable *SearchVariable(VariablesTable *table, const char *var);

VariablesTable VarsTableCtor(void);

void VarsTableDtor(VariablesTable *table);

#endif //VARIABLES_H
