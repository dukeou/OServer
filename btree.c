#include <stdlib.h>
#include <assert.h>

#include "logger.h"
#include "btree.h"

typedef enum
{
    e_insert_exist    = -1,
    e_insert_ok       = 1,
    e_insert_left     = 2,
    e_insert_right    = 3,
    e_insert_balanced = 4,
}e_insert_result_t;
void btree_init(btree_t *tree)
{
    tree->root = NULL;
}
void btree_pre_order_traverse(btree_t *tree, visit_func visit, void *data)
{
    if(tree->root)
    {
        visit(tree->root, data);
        if(tree->root->llink)
            btree_pre_order_traverse((btree_t*)&tree->root->llink, visit, data);
        if(tree->root->rlink && !tree->root->rflag)
            btree_pre_order_traverse((btree_t*)&tree->root->rlink, visit, data);
    }
}
void btree_in_order_traverse(btree_t *tree, visit_func visit, void *data)
{
    if(tree->root)
    {
        if(tree->root->llink)
            btree_in_order_traverse((btree_t*)&tree->root->llink, visit, data);
        visit(tree->root, data);
        if(tree->root->rlink && !tree->root->rflag)
            btree_in_order_traverse((btree_t*)&tree->root->rlink, visit, data);
    }
}
void btree_post_order_traverse(btree_t *tree, visit_func visit, void *data)
{
    if(tree->root)
    {
        if(tree->root->llink)
            btree_post_order_traverse((btree_t*)&tree->root->llink, visit, data);
        if(tree->root->rlink && !tree->root->rflag)
            btree_post_order_traverse((btree_t*)&tree->root->rlink, visit, data);
        visit(tree->root, data);
    }
}
void btree_visit(btree_t *tree, int16_t key, visit_func visit, void *data)
{
    bt_node_entry_t *next = tree->root;
    while(next)
    {
        if(next->key == key)
        {
            visit(next, data);
            break;
        }
        else if(key > next->key)
        {
            if(next->rflag)
                next = NULL;
            else
                next = next->rlink;
        }
        else
        {
            next = next->llink;
        }
    }
}
int btree_insert(btree_t *tree, bt_node_entry_t *node)
{
    node->llink = NULL;
    node->rlink = NULL;
    node->balance_factor = 0;
    node->rflag = 0;
    return btree_internal_insert(&tree->root, node) > 0 ? 1 : 0;
}
int btree_internal_insert(bt_node_entry_t **root, bt_node_entry_t *node)
{
    int ret;
    if(!*root)
    {
        *root = node;
        l_og(e_log_level_debug, "%d added to btree\n", node->key);
        return e_insert_ok;
    }
    if(node->key == (*root)->key)
        return e_insert_exist;
    l_og(e_log_level_debug, "before node: %p left: %p right: %p\n", *root, (*root)->llink, (*root)->rlink);
    if(node->key > (*root)->key)
    {
        l_og(e_log_level_debug, "%d > %d\n", node->key, (*root)->key);
        if((*root)->rflag)
        {
            (*root)->rflag = 0;
            (*root)->rlink = NULL;
        }
        ret = btree_internal_insert(&(*root)->rlink, node);
        switch(ret)
        {
            case e_insert_ok: case e_insert_left: case e_insert_right:
                if(--(*root)->balance_factor == -2)
                {
                    if(ret == e_insert_left)
                        btree_right_rotate(&(*root)->rlink);
                    btree_left_rotate(root);
                    assert((*root)->balance_factor == 0);
                }
                ret = (*root)->balance_factor ? e_insert_right : e_insert_balanced;
                break;
            case e_insert_exist: case e_insert_balanced: break;
        }
    }
    else
    {
        l_og(e_log_level_debug, "%d < %d\n", node->key, (*root)->key);
        ret = btree_internal_insert(&(*root)->llink, node);
        switch(ret)
        {
            case e_insert_ok: case e_insert_left: case e_insert_right:
                if(++(*root)->balance_factor == 2)
                {
                    if(ret == e_insert_right)
                        btree_left_rotate(&(*root)->llink);
                    btree_right_rotate(root);
                    assert((*root)->balance_factor == 0);
                }
                ret = (*root)->balance_factor ? e_insert_left : e_insert_balanced;
                break;
            case e_insert_exist: case e_insert_balanced: break;
        }
    }
    l_og(e_log_level_debug, "after node: %p left: %p right: %p\n", *root, (*root)->llink, (*root)->rlink);
    return ret;
}
void btree_left_rotate(bt_node_entry_t **root)
{
    l_og(e_log_level_debug, "left rotate %d, balance_factor: %d\n", (*root)->key, (*root)->balance_factor);
    bt_node_entry_t *new_root = (*root)->rlink;
    (*root)->rlink = new_root->llink;
    new_root->llink = *root;
    new_root->llink->balance_factor += new_root->balance_factor < 0 ? (1 - new_root->balance_factor) : 1;
    new_root->balance_factor += new_root->llink->balance_factor > 0 ? (1 + new_root->llink->balance_factor) : 1;
    *root = new_root;
    l_og(e_log_level_debug, "left rotate new root %d, balance_factor: %d\n", (*root)->key, (*root)->balance_factor);
    l_og(e_log_level_debug, "left rotate new root left %d, balance_factor: %d\n", (*root)->llink->key, (*root)->llink->balance_factor);
}
void btree_right_rotate(bt_node_entry_t **root)
{
    l_og(e_log_level_debug, "right rotate %d, balance_factor: %d\n", (*root)->key, (*root)->balance_factor);
    bt_node_entry_t *new_root = (*root)->llink;
    (*root)->llink = new_root->rflag ? NULL : new_root->rlink;
    new_root->rflag = 0;
    new_root->rlink = *root;
    *root = new_root;
    new_root->rlink->balance_factor -= new_root->balance_factor > 0 ? (1 + new_root->balance_factor) : 1;
    new_root->balance_factor -= new_root->rlink->balance_factor < 0 ? (1 - new_root->rlink->balance_factor) : 1;
    l_og(e_log_level_debug, "right rotate new root %d, balance_factor: %d\n", (*root)->key, (*root)->balance_factor);
    l_og(e_log_level_debug, "right rotate new root right %d, balance_factor: %d\n", (*root)->rlink->key, (*root)->rlink->balance_factor);
}
void btree_node_threading(bt_node_entry_t *node, bt_node_entry_t **pre)
{
    if(node->rflag)
    {
        node->rflag = 0;
        node->rlink = NULL;
    }
    if(*pre && (!(*pre)->rlink || (*pre)->rflag))
    {
        (*pre)->rlink = node;
        (*pre)->rflag = 1;
    }
    *pre = node;
}
void btree_build_in_order_threading(btree_t *tree)
{
    bt_node_entry_t *pre;
    pre = NULL;
    btree_in_order_traverse(tree, (visit_func)btree_node_threading, &pre);
}
void btree_thread_traverse(btree_t *tree, visit_func2 visit, void *data)
{
    bt_node_entry_t *node = tree->root;
    while(node)
    {
        while(node->llink)
            node = node->llink;
        if(visit(node, data))
            return;
        while(node->rflag)
        {
            node = node->rlink;
            if(visit(node, data))
                return;
        }
        node = node->rlink;
    }
    visit(node, data);
}
