#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "../include/treeIO.h"

static void NodeDataPrint(FILE *file, Node *node);
static void SubTreePrint(FILE *file, Node *node);

static void VariablesDump(VariablesTable *table, FILE *dump_file);

static void DotNodeCtor(Node *node, FILE *dot_file);
static void DotSubTreeCtor(Node *node, Node *node_next, const char *direction, FILE *dot_file);
static void DotTreeGeneral(Tree *tree, FILE *dot_file);





#define KEYWORD(enum, keyword, ...) case enum: {fprintf(file, keyword); return;}
static void NodeDataPrint(FILE *file, Node *node)
{
    switch(node->type)
    {
        case NUM:
        {
            fprintf(file, "%lg", node->data.num);
            return;
        }
        case VAR:
        {
            fprintf(file, "%s", node->data.var);
            return;
        }
        case KWORD:
            switch(node->data.kword)
            {
                #include "../../Keywords/functions.h"
                #include "../../Keywords/operators.h"
                default:
                {
                    fprintf(file, "unknown operator");
                    return;
                }
            }
        case UND: //fall through
        default:
        {
            fprintf(file, "unknown type");
            return;
        }
    }
}
#undef KEYWORD

static void SubTreePrint(FILE *file, Node *node)
{
    if(!node) return;

    fprintf(file, "(");

    SubTreePrint(file, node->left);
    NodeDataPrint(file, node);
    SubTreePrint(file, node->right);

    fprintf(file, ")");
}





static void VariablesDump(VariablesTable *table, FILE *dump_file)
{
    if(table)
    {
        fprintf(dump_file, "\tVariables:\n");
        for(size_t i = 0; i < table->size; i++)
        {
            fprintf(dump_file, "\t\t\'%s\' =  %lg;\n", table->vars[i].name, table->vars[i].val);
        }
    }
}

void TreeTextDump(Tree *tree, FILE *dump_file)
{
    fprintf(dump_file, "TREE[%p]:\n", tree);

    if(!tree) return;

    fprintf(dump_file, "\troot: %p; \n", tree->root);

    VariablesDump(tree->table, dump_file);
    SubTreePrint(dump_file, tree->root);

    fprintf(dump_file, "\n\n");
}




static void DotNodeCtor(Node *const node, FILE *dot_file)
{
    fprintf(dot_file, "node%p[label = \"{<data>", node);
    NodeDataPrint(dot_file, node);

    fprintf(dot_file, "| {<left> left | <right> right}}\"; fillcolor = ");

    switch(node->type)
    {
        case NUM:
        {
            fprintf(dot_file , "\"coral\"];");
            break;
        }
        case KWORD:
        {
            fprintf(dot_file , "\"orange\"];");
            break;
        }
        case VAR:
        {
            fprintf(dot_file , "\"bisque\"];");
            break;
        }
        case UND: //fall through
        default:
        {
            fprintf(dot_file , "\"red\"];");
        }
    }
}

static void DotSubTreeCtor(Node *const node, Node *const node_next, const char *const direction, FILE *dot_file)
{
    if(!node_next) return;

    DotNodeCtor(node_next, dot_file);

    fprintf(dot_file, "node%p:<%s>:s -> node%p:<data>:n;\n", node, direction, node_next);

    DotSubTreeCtor(node_next, node_next->left , "left" , dot_file);
    DotSubTreeCtor(node_next, node_next->right, "right", dot_file);
}

static void DotTreeGeneral(Tree *const tree, FILE *dot_file)
{
    fprintf(dot_file, "digraph\n"
                      "{\n"
                      "bgcolor = \"grey\";\n"
                      "node[shape = \"Mrecord\"; style = \"filled\"; fillcolor = \"#58CD36\"];\n");

    fprintf(dot_file, "{rank = source;");
    fprintf(dot_file, "nodel[label = \"<root> root: %p\"; fillcolor = \"lightblue\"]; ",
                                                                            tree->root);
    DotNodeCtor(tree->root, dot_file);

    fprintf(dot_file, "variables[label = \"%zu", tree->table->size);
    for(size_t i = 0; i < tree->table->size; i++)
    {
        fprintf(dot_file, "|{%s | %lg}", tree->table->vars[i].name, tree->table->vars[i].val);
    }
    fprintf(dot_file, "\"; fillcolor = \"lightblue\"]};\n");
}

void TreeDot(Tree *const tree, const char *png_file_name)
{
    if(!(tree && tree->root)) return;

    FILE *dot_file = fopen("tree.dot", "wb");

    DotTreeGeneral(tree, dot_file);
    DotSubTreeCtor(tree->root, tree->root->left , "left" , dot_file);
    DotSubTreeCtor(tree->root, tree->root->right, "right", dot_file);

    fprintf(dot_file, "}\n");

    fclose(dot_file);

    char *sys_cmd = NULL;
    asprintf(&sys_cmd, "dot tree.dot -T png -o %s", png_file_name);
    system(sys_cmd);
    free(sys_cmd);

    remove("tree.dot");
}