#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <pthread.h>

#include "logger.h"
#include "sync_queue.h"
#include "thread_pool.h"

int thread_pool_init(thread_pool_t *pool, int max_thread_count, int initial_thread_count)
{
    int i;
    pthread_attr_t attr;
    pool->max_count = max_thread_count;
    pool->active_count = initial_thread_count;
    pool->running_count = 0;
    pool->threads = (thread_info_t*)malloc(sizeof(thread_info_t) * max_thread_count);
    for(i = 0; i < initial_thread_count - 1; ++i)
    {
        pool->threads[i].next = &pool->threads[i + 1];
    }
    pool->threads[i].next = NULL;
    pool->pending_threads = pool->threads;
    pool->running_threads = NULL;
    pthread_mutex_init(&pool->lock, NULL);
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 10240);
    sync_queue_init(&pool->task_queue);
    for(i = 0; i < initial_thread_count; ++i)
    {
        pthread_create(&pool->threads[i].tid, &attr, thread_pool_task_entry, pool);
    }
    return 0;
}
void *thread_pool_task_entry(void *arg)
{
    thread_pool_t *pool = (thread_pool_t*)arg;
    thread_task_t *task;
    pthread_t tid;
    tid = pthread_self();
    while(1)
    {
        l_og(e_log_level_debug, "thread pending: 0x%x\n", tid);
        task = (thread_task_t*)sync_queue_pop_front(&pool->task_queue);
        if(!task)
            break;
        l_og(e_log_level_debug, "thread running: 0x%x\n", tid);
        task->run(task->arg);
        free(task);
    }
    l_og(e_log_level_debug, "thread exiting: 0x%x\n", tid);
    pthread_exit(NULL);
    return NULL;
}
void thread_pool_add_task(thread_pool_t *pool, thread_func func, void *arg)
{
    thread_task_t *new_task = (thread_task_t*)malloc(sizeof(thread_task_t));
    new_task->run = func;
    new_task->arg = arg;
    sync_queue_push_back(&pool->task_queue, (queue_entry_t*)new_task);
}
void thread_pool_destroy(thread_pool_t *pool)
{
    int i;
    thread_task_t *t;
    sync_queue_signal_destroy(&pool->task_queue);
    for(i = 0; i < pool->active_count; ++i)
    {
        pthread_join(pool->threads[i].tid, NULL);
    }
    while((t = (thread_task_t*)sync_queue_pop_front(&pool->task_queue)))
    {
        free(t);
    }
    sync_queue_destroy(&pool->task_queue);
}
