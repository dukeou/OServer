#ifndef O_LIST
#define O_LIST
typedef struct list_node
{
    struct list_node *next;
    struct list_node *prev;
}list_node_t;
typedef struct
{
    list_node_t head;
}list_t;
typedef int (*list_visit_func)(list_node_t *node, void *arg);
void list_init(list_t *list);
void list_append(list_t *list, list_node_t *node);
void list_remove(list_t *list, list_node_t *node);
void list_traverse(list_t *list, list_visit_func visit, void *arg);
int list_empty(list_t *list);
list_node_t *list_remove_first(list_t *list);
#endif /* O_LIST */
