#ifndef PARSER_H
#define PARSER_H

#include "../../general/log/log.h"
#include "../../tree/include/tree.h"

struct Token
{
    Node *lex;

    size_t line;
    size_t l_pos;
};

const char SPECIAL_CHARS[] = "()+-*/^";

Tree ParseCode(const char *file_name, VariablesTable *table);

#endif //PARSER_H