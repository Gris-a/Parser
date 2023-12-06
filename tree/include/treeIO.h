#ifndef TREE_I_O_H
#define TREE_I_O_H

#include "tree.h"

void TreeTextDump(Tree *tree, FILE *dump_file);

void TreeDot(Tree *tree, const char *png_file_name);

#endif //TREE_I_O_H