#include <stdio.h>

#include "parser/include/parser.h"
#include "tree/include/treeIO.h"
#include "tree/include/treeMath.h"

int main(void)
{
    VariablesTable table = {};
    Tree tree = ParseCode("aboba.txt", &table);

    Tree Taylor = TaylorSeries(&tree, "x", 0, 5);
    TREE_DUMP(&Taylor);

    TreeDtor(&tree, tree.root);
    TreeDtor(&Taylor, Taylor.root);
    VarsTableDtor(&table);
    return 0;
}