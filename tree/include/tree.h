#ifndef TREE_H
#define TREE_H

#include <stdbool.h>

#include "../../general/log/log.h"
#include "variables.h"

#define KEYWORD(enum_name, ...) enum_name,
enum Keyword
{
    #include "../../Keywords/functions.h"
    #include "../../Keywords/operators.h"
};
#undef KEYWORD

union data_t
{
    char      und;
    Keyword kword;
    double    num;
    char     *var;
};

enum NodeType
{
    UND   = 0,
    NUM   = 1,
    VAR   = 2,
    KWORD = 3,
};

struct Node
{
    NodeType type;
    data_t   data;

    Node *left;
    Node *right;
};

struct Tree
{
    Node *root;
    VariablesTable *table;
};

#define TREE_DUMP(tree_ptr) LOG("Called from %s:%s:%d:\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                            TreeDump(tree_ptr, __func__, __LINE__);

#ifdef PROTECT
#define TREE_VERIFICATION(tree_ptr, ret_val_on_fail) if(!IsTreeValid(tree_ptr))\
                                                     {\
                                                         LOG("%s:%s:%d: Error: invalid tree.\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                                                         TREE_DUMP(tree_ptr);\
                                                         return ret_val_on_fail;\
                                                     }
#else
#define TREE_VERIFICATION(tree_ptr, ...)
#endif

void SubTreeDtor(Node *sub_tree);

int TreeDtor(Tree *tree, Node *root);

Node *NodeCtor(data_t val, NodeType type, Node *left = NULL, Node *right = NULL);

int NodeDtor(Node *node);

Node *SubTreeCopy(Node *sub_tree);

void TreeDump(Tree *tree, const char *func, const int line);

#ifdef PROTECT
bool IsTreeValid(Tree *const tree);
#endif

#endif //TREE_H