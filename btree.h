#ifndef O_BTREE
#define O_BTREE
typedef int16_t bt_key_t;
typedef struct bt_node_entry
{
    bt_key_t             key;
    int8_t               balance_factor;
    int8_t               rflag;
    struct bt_node_entry *llink;
    struct bt_node_entry *rlink;
}bt_node_entry_t;
typedef struct
{
    bt_node_entry_t *root;
}btree_t;
typedef void (*visit_func)(bt_node_entry_t *node, void *data);
typedef int (*visit_func2)(bt_node_entry_t *node, void *data);
void btree_init(btree_t *tree);
void btree_pre_order_traverse(btree_t *tree, visit_func visit, void *data);
void btree_in_order_traverse(btree_t *tree, visit_func visit, void *data);
void btree_post_order_traverse(btree_t *tree, visit_func visit, void *data);
void btree_visit(btree_t *tree, int16_t key, visit_func visit, void *data);
int btree_insert(btree_t *tree, bt_node_entry_t *node);
int btree_internal_insert(bt_node_entry_t **root, bt_node_entry_t *node);
void btree_left_rotate(bt_node_entry_t **root);
void btree_right_rotate(bt_node_entry_t **root);
void btree_node_threading(bt_node_entry_t *node, bt_node_entry_t **pre);
void btree_build_in_order_threading(btree_t *tree);
void btree_thread_traverse(btree_t *tree, visit_func2 visit, void *data);
#endif /* O_BTREE */
