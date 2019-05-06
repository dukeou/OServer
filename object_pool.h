#ifndef O_OBJECT_POOL
#define O_OBJECT_POOL
typedef struct object
{
    struct object *next;
}object_t;
typedef struct memory
{
    struct memory *next;
}memory_t;
typedef struct
{
    object_t *available_objects;
    int object_size;
    memory_t *memptr;
}object_pool_t;
void object_pool_create(object_pool_t *op, int size, int num);
object_t *object_pool_acquire(object_pool_t *op);
void object_pool_return(object_pool_t *op, object_t *ob);
void object_pool_destroy(object_pool_t *op);
void object_pool_expand(object_pool_t *op, int num);
#endif /* O_OBJECT_POOL */
