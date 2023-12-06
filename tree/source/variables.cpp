#include <stdlib.h>
#include <string.h>

#include "../include/variables.h"

Variable *SearchVariable(VariablesTable *table, const char *var)
{
    for(size_t i = 0; i < table->size; i++)
    {
        if(strcmp(table->vars[i].name, var) == 0)
        {
            return table->vars + i;
        }
    }

    return NULL;
}

VariablesTable VarsTableCtor(void)
{
    return {};
}

void VarsTableDtor(VariablesTable *table)
{
    for(size_t i = 0; i < table->size; i++)
    {
        free(table->vars[i].name);
    }
}