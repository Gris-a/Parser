#include <stdio.h>

#include "parser/include/parser.h"
#include "tree/include/treeIO.h"

int main(void)
{
    VariablesTable table = {};
    Tree tree = ParseCode("aboba.txt", &table);
    TREE_DUMP(&tree);
    TreeDtor(&tree, tree.root);
    VarsTableDtor(&table);
    return 0;
}