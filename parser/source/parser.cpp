#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../include/parser.h"
#include "../../preprocessor/include/preprocessor.h"

static void SyntaxErrMessage(Code *code, size_t line, size_t l_pos);


static void TokensDestroy(Token *tokens, size_t n_tok);
static void TokensClear(Token *tokens);


static bool   SkipSpaces        (const char *str, size_t *pos);
static bool   IsSpecialCharacter(char ch);
static double GetDouble         (const char *str, size_t *pos);
static void   GetIdentificator  (const char *str, size_t *pos, char buffer[]);

static Token *Tokenizator(Code *code, VariablesTable *table, size_t *n_tok);


static Node *ParseExprAddSub(Token *tokens, size_t *t_pos, Code *code);
static Node *ParseExprMulDiv(Token *tokens, size_t *t_pos, Code *code);
static Node *ParseExprPow   (Token *tokens, size_t *t_pos, Code *code);
static Node *ParsePrimary   (Token *tokens, size_t *t_pos, Code *code);
static Node *ParseBrackets  (Token *tokens, size_t *t_pos, Code *code);





static void SyntaxErrMessage(Code *code, size_t line, size_t l_pos)
{
    LOG("Syntax error at line %zu:\n", line);
    LOG("%s\n", code->lines[line - 1]);
    LOG("%*s\n", (int)l_pos, "^");
}


static void TokensDestroy(Token *tokens, size_t n_tok)
{
    for(size_t i = 0; i < n_tok; i++)
    {
        SubTreeDtor(tokens[i].lex);
    }
    free(tokens);
}

static void TokensClear(Token *tokens)
{
    size_t pos = 0;
    while(true)
    {
        if((tokens[pos].lex->type == UND))
        {
            if(tokens[pos].lex->data.und == '\0')
            {
                break;
            }

            SubTreeDtor(tokens[pos].lex);
        }

        pos++;
    }
    SubTreeDtor(tokens[pos].lex);
}


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
                                    tokens[(*n_tok)++] = {.lex = NodeCtor({.kword = enum}, KWORD), .line = line, .l_pos = old_pos};\
                                } else
#define LINE code->lines[line - 1]
#define CHAR code->lines[line - 1][pos]
static Token *Tokenizator(Code *code, VariablesTable *table, size_t *n_tok)
{
    size_t N_TOKENS = 100;
    Token *tokens = (Token *)calloc(N_TOKENS, sizeof(Token));

    size_t line = 1;
    size_t pos  = 0;
    for(; line <= code->n_lines; line++)
    {
        pos = 0;
        while(CHAR != '\0')
        {
            if((*n_tok) == N_TOKENS)
            {
                tokens = (Token *)realloc(tokens, (N_TOKENS *= 2) * sizeof(Token));
            }

            bool is_end = SkipSpaces(LINE, &pos);
            if(is_end) break;

            size_t old_pos = pos;
            if(isdigit(CHAR))
            {
                double dbl      = GetDouble(LINE, &pos);
                tokens[(*n_tok)++] = {.lex = NodeCtor({.num = dbl}, NUM), .line  = line,
                                                                          .l_pos = old_pos};
            }
            else
            {
                char name_buf[BUFSIZ / 16] = {};
                bool is_special = false;

                if((is_special = IsSpecialCharacter(CHAR)))
                {
                    name_buf[0] = CHAR;
                    pos++;
                }
                else if((isalpha(CHAR)) || (CHAR == '_'))
                {
                    GetIdentificator(LINE, &pos, name_buf);
                }
                else
                {
                    SyntaxErrMessage(code, line, old_pos);
                    TokensDestroy(tokens, (*n_tok));
                    return NULL;
                }

                #include "../../Keywords/functions.h"
                #include "../../Keywords/operators.h"
                /*else*/if(is_special)
                {
                    tokens[(*n_tok)++] = {.lex = NodeCtor({.und = name_buf[0]}, UND), .line  = line,
                                                                                      .l_pos = old_pos};
                }
                else
                {
                    Variable *var = SearchVariable(table, name_buf);

                    if(table->size == MAX_VARIABLES)
                    {
                        LOG("Too much variables.\n");
                        TokensDestroy(tokens, (*n_tok));
                        return NULL;
                    }

                    if(!var)
                    {
                        table->vars[table->size].name = strdup(name_buf);

                        var = table->vars + table->size;
                        table->size++;
                    }

                    tokens[(*n_tok)++] = {.lex = NodeCtor({.var = var->name}, VAR), .line  = line,
                                                                                    .l_pos = old_pos};
                }
            }
        }
    }
    tokens[(*n_tok)++] = {.lex = NodeCtor({.und = '\0'}, UND), .line  = line - 1,
                                                               .l_pos = pos};
    tokens = (Token *)realloc(tokens, (*n_tok) * sizeof(Token));

    return tokens;
}
#undef KEYWORD
#undef LINE
#undef CHAR


Tree ParseCode(const char *file_name, VariablesTable *table)
{
    Tree tree = {.root = NULL, .table = table};

    __attribute__((cleanup(BufDtor)))  Buffer buf = {};
    __attribute__((cleanup(CodeDtor))) Code code  = ReadCode(file_name, &buf);

    if(!code.lines) return {};

    size_t t_pos = 0;
    size_t n_tok = 0;

    Token *tokens = Tokenizator(&code, table, &n_tok);

    if(!tokens) return {};

    tree.root = ParseExprAddSub(tokens, &t_pos, &code);

    if(!(tree.root) || !((tokens[t_pos].lex->type     == UND) &&
                         (tokens[t_pos].lex->data.und == '\0')))
    {
        SyntaxErrMessage(&code, tokens[t_pos].line, tokens[t_pos].l_pos);
        TokensDestroy(tokens, n_tok);

        return {};
    }

    TokensClear(tokens);
    free(tokens);

    return tree;
}


static Node *ParseExprAddSub(Token *tokens, size_t *t_pos, Code *code)
{
    Node *sign = tokens[*t_pos].lex;
    if(sign->type == KWORD && ((sign->data.kword == ADD) ||
                               (sign->data.kword == SUB)))
        (*t_pos)++;
    else
        sign = NULL;

    Node *ret_val = ParseExprMulDiv(tokens, t_pos, code);
    if(!ret_val) return NULL;

    if(sign)
    {
        sign->left  = NodeCtor({.num = 0}, NUM);
        sign->right = ret_val;
        ret_val     = sign;
    }

    Node *temp = NULL;
    Node *op   = tokens[*t_pos].lex;

    while(op->type == KWORD && ((op->data.kword == ADD) || (op->data.kword == SUB)))
    {
        (*t_pos)++;

        temp = ParseExprMulDiv(tokens, t_pos, code);
        if(!temp) return NULL;

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
        if(!temp) return NULL;

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
        if(!temp) return NULL;

        op->left  = ret_val;
        op->right = temp;
        ret_val   = op;

        op = tokens[*t_pos].lex;
    }

    return ret_val;
}

#define KEYWORD(enum, keyword) case enum: break;
static Node *ParsePrimary(Token *tokens, size_t *t_pos, Code *code)
{
    Node *ret_val = tokens[*t_pos].lex;

    switch(ret_val->type)
    {
        case VAR: (*t_pos)++; break;
        case NUM: (*t_pos)++; break;
        case KWORD:
        {
            switch(ret_val->data.kword)
            {
                #include "../../Keywords/functions.h"
                default: return NULL;
            }

            (*t_pos)++;

            ret_val->right = ParseBrackets(tokens, t_pos, code);
            if(!ret_val->right) return NULL;

            break;
        }
        case UND:
        {
            ret_val = ParseBrackets(tokens, t_pos, code);

            break;
        }
        default: return NULL;
    }

    return ret_val;
}
#undef KEYWORD

static Node *ParseBrackets(Token *tokens, size_t *t_pos, Code *code)
{
    if(!((tokens[*t_pos].lex->type == UND) && (tokens[*t_pos].lex->data.und == '(')))
    {
        return NULL;
    }
    (*t_pos)++;

    Node *ret_val = ParseExprAddSub(tokens,t_pos, code);

    if(!((tokens[*t_pos].lex->type == UND) && (tokens[*t_pos].lex->data.und == ')')))
    {
        return NULL;
    }
    (*t_pos)++;

    return ret_val;
}