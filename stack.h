#ifndef O_HTTP_STACK
#define O_HTTP_STACK
#include "list.h"
typedef list_t o_stack_t;
typedef list_node_t stack_node_t;
typedef list_visit_func stack_visit_func;
#define stack_init list_init
#define stack_push list_append
#define stack_empty list_empty
#define stack_traverse list_traverse
stack_node_t *stack_pop(o_stack_t *stack);
stack_node_t *stack_top(o_stack_t *stack);
#endif /* O_HTTP_URL */
