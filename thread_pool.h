#ifndef O_THREAD_POOL
#define O_THREAD_POOL
typedef void (*thread_func)(void*);
typedef struct thread_task
{
    struct thread_task *next;
    thread_func run;
    void *arg;
}thread_task_t;
typedef struct thread_info
{
    struct thread_info *next;
    pthread_t tid;
}thread_info_t;
typedef struct
{
    int max_count;
    int active_count;
    int running_count;
    sync_queue_t task_queue;
    thread_info_t *threads;
    thread_info_t *pending_threads;
    thread_info_t *running_threads;
    pthread_mutex_t lock;
}thread_pool_t;
int thread_pool_init(thread_pool_t *pool, int max_thread_count, int initial_thread_count);
void *thread_pool_task_entry(void *arg);
void thread_pool_add_task(thread_pool_t *pool, thread_func func, void *arg);
void thread_pool_destroy(thread_pool_t *pool);
#endif /* O_THREAD_POOL */
