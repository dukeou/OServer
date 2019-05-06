#ifndef O_SYNC_QUEUE
#define O_SYNC_QUEUE
typedef struct queue_entry
{
    struct queue_entry *next;
}queue_entry_t;
typedef struct
{
    queue_entry_t *head;
    queue_entry_t *tail;
    int destorying;
    pthread_mutex_t lock;
    pthread_cond_t condition;
}sync_queue_t;
void sync_queue_init(sync_queue_t *queue);
void sync_queue_push_back(sync_queue_t *queue, queue_entry_t *ele);
queue_entry_t *sync_queue_pop_front(sync_queue_t *queue);
void sync_queue_signal_destroy(sync_queue_t *queue);
void sync_queue_destroy(sync_queue_t *queue);
#endif /* O_SYNC_QUEUE */
