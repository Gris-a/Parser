#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "../../general/log/log.h"

struct Buffer
{
    char *buf;

    size_t size;
    size_t n_lines;
};

struct Code
{
    char **lines;

    size_t n_lines;
};

int BufCode(Buffer *buf, const char *file_name);

void BufDtor(Buffer *buf);

Code ReadCode(const char *file_name, Buffer *buf);

void CodeDtor(Code *code);

#endif //PREPROCESSOR_H