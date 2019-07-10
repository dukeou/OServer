#include <stdlib.h>

#include "stack.h"

stack_node_t *stack_pop(o_stack_t *stack)
{
    if(stack_empty(stack))
    {
        return NULL;
    }
    stack_node_t *node = stack->head.prev;
    node->prev->next = &stack->head;
    stack->head.prev = node->prev;
    return node;
}
stack_node_t *stack_top(o_stack_t *stack)
{
    if(stack_empty(stack))
    {
        return NULL;
    }
    stack_node_t *node = stack->head.prev;
    return node;
}
