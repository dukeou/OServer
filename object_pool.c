#include <stdlib.h>

#include "object_pool.h"
void object_pool_create(object_pool_t *op, int size, int num)
{
    int len;
    char *mem, *end;
    len = size * num;
    mem = (char*)malloc(len + sizeof(memory_t));
    op->memptr = (memory_t*)mem;
    op->memptr->next = NULL;
    op->object_size = size;
    mem += sizeof(memory_t);
    op->available_objects = (object_t*)mem;
    end = mem + len - size;
    for(; mem < end; mem += size)
        ((object_t*)mem)->next = (object_t*)(mem + size);
    ((object_t*)mem)->next = NULL;
}
object_t *object_pool_acquire(object_pool_t *op)
{
    object_t *ob = NULL;
    if(op->available_objects)
    {
        ob = op->available_objects;
        op->available_objects = ob->next;
    }
    return ob;
}
void object_pool_return(object_pool_t *op, object_t *ob)
{
    if(!op->available_objects)
    {
        ob->next = NULL;
        op->available_objects = ob;
    }
    else
    {
        ob->next = op->available_objects;
        op->available_objects = ob;
    }
}
void object_pool_destroy(object_pool_t *op)
{
    memory_t *mem, *ptr;
    ptr = op->memptr;
    while(ptr)
    {
        mem = ptr;
        ptr = ptr->next;
        free(mem);
    }
    op->memptr = NULL;
    op->available_objects = NULL;
}
void object_pool_expand(object_pool_t *op, int num)
{
    int i;
    char *mem;
    object_t *optr;
    mem = (char*)malloc(op->object_size * num + sizeof(memory_t));
    ((memory_t*)mem)->next = op->memptr;
    op->memptr = (memory_t*)mem;
    optr = (object_t*)(mem + sizeof(memory_t));
    for(i = 0; i < num - 1; ++i)
        optr[i].next = &optr[i + 1];
    optr[i].next = op->available_objects;
    op->available_objects = optr;
}
