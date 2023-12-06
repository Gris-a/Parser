#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../include/preprocessor.h"

static size_t FileSize(const char *file_name)
{
    struct stat file_info = {};
    stat(file_name, &file_info);

    return (size_t)file_info.st_size;
}

int BufCode(Buffer *buf, const char *file_name)
{
    FILE *file = fopen(file_name, "rb");
    if(!file)
    {
        LOG("No such file: \"%s\"\n", file_name);
        return EXIT_FAILURE;
    }

    buf->size = FileSize(file_name);
    buf->buf  = (char *)calloc(buf->size + 1, sizeof(char));

    fread(buf->buf, sizeof(char), buf->size, file);
    fclose(file);

    return EXIT_SUCCESS;
}


void BufDtor(Buffer *buf)
{
    free(buf->buf);
    buf->buf = NULL;

    buf->size = 0;
    buf->n_lines   = 0;
}


static void CodePreprocessing(Buffer *buf)
{
    buf->n_lines = 0;

    if(*buf->buf == '\0') return;

    char *read_p  = buf->buf + 1;
    char *write_p = buf->buf + 1;

    if(isspace(buf->buf[0])) write_p--;

    for(; *read_p != '\0'; read_p++)
    {
        if(*read_p == '\n')
        {
            if(*(read_p - 1) == '\n') continue;

            buf->n_lines++;
        }

        *(write_p++) = *read_p;
    }
    *write_p = '\0';

    if(read_p[-1] != '\n') buf->n_lines++;

    buf->size = (size_t)(write_p - buf->buf + 1);
    buf->buf  = (char *)realloc(buf->buf, buf->size);
}

static void CodeDelim(Code *code, Buffer *buf)
{
    char *buf_p = buf->buf;

    code->lines[0] = buf_p;
    size_t line_count = 1;

    for(; *buf_p != '\0'; buf_p++)
    {
        if(*buf_p == '\n' && *(buf_p + 1) != '\0')
        {
            *(buf_p++) = '\0';

            code->lines[line_count++] = buf_p;
        }
    }
}

Code ReadCode(const char *file_name, Buffer *buf)
{
    if(!buf->buf)
    {
        int exit_status = BufCode(buf, file_name);
        if(exit_status == EXIT_FAILURE)
        {
            return {};
        }
    }

    CodePreprocessing(buf);

    Code code = {};

    if(buf->n_lines != 0)
    {
        code.n_lines = buf->n_lines;
        code.lines   = (char **)calloc(code.n_lines, sizeof(char *));

        CodeDelim(&code, buf);
    }

    return code;
}


void CodeDtor(Code *code)
{
    free(code->lines);
    code->lines = NULL;

    code->n_lines = 0;
}