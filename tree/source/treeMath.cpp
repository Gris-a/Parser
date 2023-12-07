#include <math.h>
#include <time.h>
#include <string.h>

#include "../include/treeMath.h"

static double SubTreeCalculate(Tree *tree, Node *node);

static Node *SubTreeDerivative(Node *node, const char *var);

// static bool SubTreeSearchVar(Node *const node, VariablesTable *table);
// static void SubTreeSimplify(Tree *tree, Node *node, bool *changes);






#define KEYWORD(enum, keyword, eval, ...) case enum: eval
static double SubTreeCalculate(Tree *tree, Node *node)
{
    if(!node) return NAN;

    switch(node->type)
    {
        case KWORD:
        {
            double lvalue = SubTreeCalculate(tree, node->left);
            double rvalue = SubTreeCalculate(tree, node->right);

            switch(node->data.kword)
            {
                #include "../../Keywords/functions.h"
                #include "../../Keywords/operators.h"
                default: return NAN;
            }
        }
        case NUM:
            return node->data.num;
        case VAR:
        {
            Variable *var = SearchVariable(tree->table, node->data.var);
            return var->val;
        }
        case UND:
        default: return NAN;
    }
}
#undef KEYWORD

double TreeCalculate(Tree *tree)
{
    TREE_VERIFICATION(tree, NAN);

    double result = SubTreeCalculate(tree, tree->root);
    ASSERT(isfinite(result), return NAN);

    return result;
}


#define KEYWORD(enum, keyword, eval, diff, ...) case enum: diff
static Node *SubTreeDerivative(Node *node, const char *var)
{
    ASSERT(node, return NULL);

    Node *ans = NULL;
    switch(node->type)
    {
        case KWORD:
        {
            switch(node->data.kword)
            {
                #include "../../Keywords/functions.h"
                #include "../../Keywords/operators.h"
                default:
                {
                    LOG("Unknown keyword.\n");
                    return NULL;
                }
            }
            break;
        }
        case NUM:
        {
            ans = __NUM(0);
            break;
        }
        case VAR:
        {
            ans = __NUM(1); //TODO
            break;
        }
        case UND: //fall through
        default:
        {
            LOG("Unknown node type.\n");
            return NULL;
        }
    }

    return ans;
}
#undef KEYWORD

Tree Derivative(Tree *const tree, const char *const var)
{
    TREE_VERIFICATION(tree, {});

    Tree deriv  = {.root  = NULL,
                   .table = tree->table};

    if(!SearchVariable(tree->table, var))
    {
        deriv.root = NodeCtor({.num = 0}, NUM);
    }
    else
    {
        deriv.root = SubTreeDerivative(tree->root, var);
    }

    return deriv;
}


Tree TaylorSeries(Tree *tree, const char *var_name, double point, size_t n)
{
    TREE_VERIFICATION(tree, {});

    Variable *var = SearchVariable(tree->table, var_name);
    if(!var) return {};

    double val_prev = var->val;
    var->val        = point;

    Tree Taylor = {.root  = __NUM(TreeCalculate(tree)),
                   .table = tree->table};

    Tree n_deriv = {.root  = SubTreeCopy(tree->root),
                    .table = tree->table};
    Tree temp = n_deriv;

    size_t factorial = 1;

    for(size_t power = 1; power <= n; power++)
    {
        factorial *= power;

        TreeSimplify(&temp);
        n_deriv = Derivative(&temp, var_name);

        Taylor.root = __ADD(Taylor.root,
                            __MUL(__NUM(TreeCalculate(&n_deriv) / (double)factorial),
                                  __POW(__SUB(__VAR(var->name), __NUM(var->val)),
                                        __NUM((double)power))));

        TreeDtor(&temp, temp.root);
        temp = n_deriv;
    }

    TreeDtor(&n_deriv, n_deriv.root);
    TreeSimplify(&Taylor);

    var->val = val_prev;

    return Taylor;
}


static bool SubTreeSearchVar(Node *node, VariablesTable *table)
{
    if(!node) return false;

    switch(node->type)
    {
        case VAR:
            return true;
        case KWORD:
            return (SubTreeSearchVar(node->left , table) ||
                    SubTreeSearchVar(node->right, table));
        case NUM:
        case UND:
        default: return false;
    }
}

#define KEYWORD(enum, keyword, eval, diff, simp) case enum: simp
static void SubTreeSimplify(Tree *tree, Node *node, bool *changes)
{
    if(!node) return;

    switch(node->type)
    {
        case KWORD:
        {
            Node *copy = NULL;

            if(!SubTreeSearchVar(node, tree->table))
            {
                copy = __NUM(SubTreeCalculate(tree, node));
            }
            else
            {
                switch(node->data.kword)
                {
                    #include "../../Keywords/functions.h"
                    #include "../../Keywords/operators.h"
                    default: return;
                }
            }

            if(copy)
            {
                *changes = true;

                TreeDtor(tree, node->left);
                TreeDtor(tree, node->right);

                *node = *copy;
                free(copy);

                SubTreeSimplify(tree, node, changes);
            }
            else
            {
                SubTreeSimplify(tree, node->left , changes);
                SubTreeSimplify(tree, node->right, changes);
            }
        }
        case NUM:
        case VAR:
        case UND:
        default: return;
    }
}
#undef KEYWORD


int TreeSimplify(Tree *tree)
{
    TREE_VERIFICATION(tree, EXIT_FAILURE);

    bool changes = false;
    while(true)
    {
        changes = false;
        SubTreeSimplify(tree, tree->root, &changes);
        if(!changes) break;
    }

    return EXIT_SUCCESS;
}