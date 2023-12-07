#ifndef MATH_DSL_H
#define MATH_DSL_H

//General
#define __DIF(node) SubTreeDerivative(node, var)
#define __CPY(node) SubTreeCopy(node)

#define NUM_T(node)    (node->type == NUM)
#define VAR_T(node)    (node->type == VAR)
#define KWORD_T(node)  (node->type == KWORD)

#define LEFT  node->left
#define RIGHT node->right


#define VARCMP(var1, var2) (strcmp(var1, var2) == 0)

#define DBLCMP(num1, num2) (abs(num1 - num2) < M_ERR)



//Node generator
#define __NUM(value)  NodeCtor({.num = (value)}, NUM)
#define __VAR(varia)  NodeCtor({.var = (varia)}, VAR)

#define __ADD(...)    NodeCtor({.kword = ADD }, KWORD, __VA_ARGS__)
#define __SUB(...)    NodeCtor({.kword = SUB }, KWORD, __VA_ARGS__)
#define __MUL(...)    NodeCtor({.kword = MUL }, KWORD, __VA_ARGS__)
#define __DIV(...)    NodeCtor({.kword = DIV }, KWORD, __VA_ARGS__)
#define __POW(...)    NodeCtor({.kword = POW }, KWORD, __VA_ARGS__)
#define __SIN(node)   NodeCtor({.kword = SIN }, KWORD, NULL, node )
#define __COS(node)   NodeCtor({.kword = COS }, KWORD, NULL, node )
#define __LN(node)    NodeCtor({.kword = LN  }, KWORD, NULL, node )

#endif //MATH_DSL_H