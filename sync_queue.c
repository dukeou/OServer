#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <pthread.h>

#include "logger.h"
#include "sync_queue.h"

void sync_queue_init(sync_queue_t *queue)
{
    queue->head = NULL;
    queue->tail = NULL;
    queue->destorying = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->condition, NULL);
}
void sync_queue_push_back(sync_queue_t *queue, queue_entry_t *ele)
{
    ele->next = NULL;
    pthread_mutex_lock(&queue->lock);
    l_og(e_log_level_debug, "sync_queue_push_back enter\n");
    if(!queue->destorying)
    {
        if(!queue->tail)
        {
            queue->tail = ele;
            queue->head = ele;
        }
        else
        {
            queue->tail->next = ele;
            queue->tail = ele;
        }
        pthread_cond_signal(&queue->condition);
    }
    l_og(e_log_level_debug, "sync_queue_push_back exit\n");
    pthread_mutex_unlock(&queue->lock);
}
queue_entry_t *sync_queue_pop_front(sync_queue_t *queue)
{
    queue_entry_t *ret = NULL;
    pthread_mutex_lock(&queue->lock);
    l_og(e_log_level_debug, "sync_queue_pop_front enter\n");
    while(!queue->destorying && !queue->head)
    {
        pthread_cond_wait(&queue->condition, &queue->lock);
    }
    if(queue->head)
    {
        ret = queue->head;
        queue->head = queue->head->next;
        if(queue->tail == ret)
            queue->tail = queue->head;
    }
    l_og(e_log_level_debug, "sync_queue_pop_front exit\n");
    pthread_mutex_unlock(&queue->lock);
    return ret;
}
void sync_queue_signal_destroy(sync_queue_t *queue)
{
    queue->destorying = 1;
    pthread_cond_broadcast(&queue->condition);
}
void sync_queue_destroy(sync_queue_t *queue)
{
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->condition);
}
