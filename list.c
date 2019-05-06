#include <stdlib.h>

#include "list.h"
void list_init(list_t *list)
{
    list->head.next = &list->head;
    list->head.prev = &list->head;
}
void list_append(list_t *list, list_node_t *node)
{
    node->next = &list->head;
    node->prev = list->head.prev;
    list->head.prev->next = node;
    list->head.prev = node;
}
void list_remove(list_t *list, list_node_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}
void list_traverse(list_t *list, list_visit_func visit, void *arg)
{
    list_node_t *node, *tmp;
    node = list->head.next;
    while(node != &list->head)
    {
        tmp = node->next;
        if(!visit(node, arg))
            break;
        node = tmp;
    }
}
int list_empty(list_t *list)
{
    return list->head.next == &list->head ? 1 : 0;
}
list_node_t *list_remove_first(list_t *list)
{
    list_node_t *node = NULL;
    if(!list_empty(list))
    {
        node = list->head.next;
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    return node;
}
