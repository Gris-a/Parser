#include <stdio.h>
#include <stdlib.h>

#include "../include/tree.h"
#include "../include/treeIO.h"

static Node *SubTreeSearchParent(Node *node, Node *search_node);

static void MakeDumpDir(void);


static Node *SubTreeSearchParent(Node *node, Node *search_node)
{
    if(!node) return NULL;
    else if(node->left  == search_node ||
            node->right == search_node) return node;

    Node *find  = SubTreeSearchParent(node->left , search_node);

    if(find == NULL)
    {
        find = SubTreeSearchParent(node->right, search_node);
    }

    return find;
}

void SubTreeDtor(Node *sub_tree)
{
    if(!sub_tree) return;

    SubTreeDtor(sub_tree->left);
    SubTreeDtor(sub_tree->right);

    NodeDtor(sub_tree);
}

int TreeDtor(Tree *tree, Node *root)
{
    TREE_VERIFICATION(tree, EXIT_FAILURE);
    if(!root) return EXIT_FAILURE;

    if(root->left)
    {
        SubTreeDtor(root->left);
        root->left  = NULL;
    }
    if(root->right)
    {
        SubTreeDtor(root->right);
        root->right = NULL;
    }

    if(root == tree->root)
    {
        tree->root  = NULL;
        tree->table = NULL;
    }
    else
    {
        Node *parent = SubTreeSearchParent(tree->root, root);

        if(parent->left == root) parent->left  = NULL;
        else                     parent->right = NULL;
    }

    NodeDtor(root);

    return EXIT_SUCCESS;
}


Node *NodeCtor(data_t val, NodeType type, Node *left, Node *right)
{
    Node *node = (Node *)calloc(1, sizeof(Node));

    node->type  = type;
    node->data  = val;

    node->left  = left;
    node->right = right;

    return node;
}

int NodeDtor(Node *node)
{
    ASSERT(node, return EXIT_FAILURE);
    free(node);

    return EXIT_SUCCESS;
}


Node *SubTreeCopy(Node *sub_tree)
{
    if(!sub_tree) return NULL;
    return NodeCtor(sub_tree->data, sub_tree->type, SubTreeCopy(sub_tree->left),
                                                    SubTreeCopy(sub_tree->right));
}


static void MakeDumpDir(void)
{
    system("rm -rf dump_tree");
    system("mkdir dump_tree");
}

void TreeDump(Tree *tree, const char *func, const int line)
{
    static int num = 0;

    if(num == 0) MakeDumpDir();

    TreeTextDump(tree, LOG_FILE);

    char *file_name = NULL;

    asprintf(&file_name, "dump_tree/tree_dump%d__%s:%d__.png", num, func, line);
    TreeDot(tree, file_name);
    free(file_name);

    num++;
}

#ifdef PROTECT
//TODO upgrade
bool IsTreeValid(Tree *const tree)
{
    ASSERT(tree && tree->root, return false);
    ASSERT(tree->table       , return false);

    return true;
}
#endif