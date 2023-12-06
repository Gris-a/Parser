#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../include/parser.h"
#include "../../preprocessor/include/preprocessor.h"

static bool   SkipSpaces        (const char *str, size_t *pos);
static bool   IsSpecialCharacter(char ch);
static double GetDouble         (const char *str, size_t *pos);
static void   GetIdentificator  (const char *str, size_t *pos, char buffer[]);

static Token *Tokenizator(Code *code, VariablesTable *table);





static Node *ParseExprAddSub(Token *tokens, size_t *t_pos, Code *code);
static Node *ParseExprMulDiv(Token *tokens, size_t *t_pos, Code *code);
static Node *ParseExprPow   (Token *tokens, size_t *t_pos, Code *code);
static Node *ParsePrimary   (Token *tokens, size_t *t_pos, Code *code);



//TODO syntax err message and don't forget about error at the end of the code('\0' check will tell that all fine)
//TODO free tokens when needed

static void GetIdentificator(const char *str, size_t *pos, char buffer[])
{
    size_t buf_pos = 0;
    while(isalpha(str[*pos]) || (str[*pos] == '_'))
    {
        buffer[buf_pos++] = str[(*pos)++];
    }
}

static double GetDouble(const char *str, size_t *pos)
{
    char *end_p   = NULL;
    double result = strtod(str + *pos, &end_p);

    *pos = (size_t)(end_p - str);
    return result;
}

static bool IsSpecialCharacter(char ch)
{
    for(size_t i = 0; SPECIAL_CHARS[i] != '\0'; i++)
    {
        if(SPECIAL_CHARS[i] == ch) return true;
    }
    return false;
}

static bool SkipSpaces(const char *str, size_t *pos)
{
    while(isspace(str[*pos])) (*pos)++;
    return (str[*pos] == '\0');
}


#define KEYWORD(enum, keyword)  if(strcmp(keyword, name_buf) == 0)\
                                {\
                                    tokens[t_pos++] = {.lex = NodeCtor({.kword = enum}, KWORD), .line = line, .l_pos = old_pos};\
                                } else
static Token *Tokenizator(Code *code, VariablesTable *table)
{
    size_t N_TOKENS = 100;
    Token *tokens = (Token *)calloc(N_TOKENS, sizeof(Token));
    size_t t_pos  = 0;

    size_t line = 0;
    size_t pos  = 0;
    for(; line < code->n_lines; line++)
    {
        pos = 0;
        while(code->lines[line][pos] != '\0')
        {
            if(t_pos == N_TOKENS)
            {
                tokens = (Token *)realloc(tokens, N_TOKENS *= 2);
            }

            bool is_end = SkipSpaces(code->lines[line], &pos);
            if(is_end) break;

            size_t old_pos = pos;
            if(isdigit(code->lines[line][pos]))
            {
                double dbl = GetDouble(code->lines[line], &pos);

                tokens[t_pos++] = {.lex = NodeCtor({.num = dbl}, NUM), .line = line, .l_pos = old_pos};
            }
            else
            {
                char name_buf[BUFSIZ / 16] = {};
                bool is_special = false;

                if((is_special = IsSpecialCharacter(code->lines[line][pos])))
                {
                    name_buf[0] = code->lines[line][pos++];
                }
                else if((isalpha(code->lines[line][pos])) || (code->lines[line][pos] == '_'))
                {
                    GetIdentificator(code->lines[line], &pos, name_buf);
                }
                else
                {
                    LOG("syntax error.\n");//TODO function for deez and tokens free

                    return NULL;
                }

                #include "../../general/Keywords.h"
                /*else*/if(is_special)
                {
                    tokens[t_pos++] = {.lex = NodeCtor({.und = name_buf[0]}, UND), .line = line, .l_pos = old_pos};
                }
                else
                {
                    Variable *var = SearchVariable(table, name_buf);

                    if(!var)
                    {
                        table->vars[table->size].name = strdup(name_buf);
                        var = table->vars + (table->size++);
                    }

                    tokens[t_pos++] = {.lex = NodeCtor({.var = var->name}, VAR), .line = line, .l_pos = old_pos};
                }
            }
        }
    }

    tokens[t_pos++] = {.lex = NodeCtor({.und = '\0'}, UND), .line = line, .l_pos = pos};
    // tokens = (Token *)realloc(tokens, t_pos);

    return tokens;
}


Tree ParseCode(const char *file_name, VariablesTable *table)
{
    Buffer buf  = {};
    Code code = ReadCode(file_name, &buf);
    if(!code.lines) return {};

    size_t t_pos  = 0;
    Token *tokens = Tokenizator(&code, table);
    if(!tokens) return {};

    Node *root = ParseExprAddSub(tokens, &t_pos, &code);
    if(!((tokens[t_pos].lex->type     == UND) &&
         (tokens[t_pos].lex->data.und == '\0'))) return {};

    Tree tree  = {};
    tree.root  = root;
    tree.table = table;

    BufDtor(&buf);
    CodeDtor(&code);
    free(tokens);

    return tree;
}



static Node *ParseExprAddSub(Token *tokens, size_t *t_pos, Code *code)
{
    int sign = 0;
    if((tokens[*t_pos].lex->type == KWORD) && (tokens[*t_pos].lex->data.kword == ADD)) {sign = -1; (*t_pos)++;}
    if((tokens[*t_pos].lex->type == KWORD) && (tokens[*t_pos].lex->data.kword == SUB)) {sign = -1; (*t_pos)++;}

    Node *ret_val = ParseExprMulDiv(tokens, t_pos, code);
    if(!ret_val) return NULL;

    ret_val->data.num *= sign;

    Node *temp = NULL;
    Node *op   = tokens[*t_pos].lex;

    while(op->type == KWORD && ((op->data.kword == ADD) || (op->data.kword == SUB)))
    {
        (*t_pos)++;

        temp = ParseExprMulDiv(tokens, t_pos, code);
        if(!temp) break;

        op->left  = ret_val;
        op->right = temp;
        ret_val   = op;

        op = tokens[*t_pos].lex;
    }

    return ret_val;
}

static Node *ParseExprMulDiv(Token *tokens, size_t *t_pos, Code *code)
{
    Node *ret_val = ParseExprPow(tokens, t_pos, code);
    if(!ret_val) return NULL;

    Node *temp = NULL;
    Node *op   = tokens[*t_pos].lex;

    while(op->type == KWORD && ((op->data.kword == MUL) || (op->data.kword == DIV)))
    {
        (*t_pos)++;

        temp = ParseExprPow(tokens, t_pos, code);
        if(!temp) break;

        op->left  = ret_val;
        op->right = temp;
        ret_val   = op;

        op = tokens[*t_pos].lex;
    }

    return ret_val;
}

static Node *ParseExprPow(Token *tokens, size_t *t_pos, Code *code)//TODO stack maybe
{
    Node *ret_val = ParsePrimary(tokens, t_pos, code);
    if(!ret_val) return NULL;

    Node *temp = NULL;
    Node *op   = tokens[*t_pos].lex;

    while((op->type == KWORD) && (op->data.kword == POW))
    {
        (*t_pos)++;

        temp = ParsePrimary(tokens, t_pos, code);
        if(!temp) break;

        op->left  = ret_val;
        op->right = temp;
        ret_val   = op;

        op = tokens[*t_pos].lex;
    }

    return ret_val;
}

static Node *ParsePrimary(Token *tokens, size_t *t_pos, Code *code)
{
    Node *ret_val = tokens[*t_pos].lex;
    (*t_pos)++;

    switch(ret_val->type)
    {
        case VAR: break;
        case NUM: break;
        case KWORD:
        {
            if(!((tokens[*t_pos].lex->type     == UND) &&
                 (tokens[*t_pos].lex->data.und == '('))) return NULL;
            ret_val->right = ParsePrimary(tokens, t_pos, code);

            break;
        }
        case UND:
        {
            if(!(ret_val->data.und == '(')) return NULL;

            ret_val = ParseExprAddSub(tokens, t_pos, code);

            if(!((tokens[*t_pos].lex->type     == UND) &&
                 (tokens[*t_pos].lex->data.und == ')'))) return NULL;
            (*t_pos)++;

            break;
        }
        default: return NULL;
    }

    return ret_val;
}