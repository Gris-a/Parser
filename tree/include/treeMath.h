#ifndef TREE_MATH_H
#define TREE_MATH_H

#include "../../general/log/log.h"
#include "tree.h"
#include "DSL.h"

const double M_ERR = 1e-20;

double TreeCalculate(Tree *tree);

Tree Derivative(Tree *tree, const char *var);

Tree TaylorSeries(Tree *tree, const char *var_name, double point, size_t n);

int TreeSimplify(Tree *tree);

#endif //TREE_MATH_H