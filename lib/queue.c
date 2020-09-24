#include "queue.h"

#include <pthread.h>
#include <stdlib.h>

Queue* init_queue(int size) {
    if (size == 0) {
        size = DEFAULT_SIZE;
    }

    Queue* q = malloc(sizeof(Queue));
    pthread_rwlock_init(&(q->q_lock), NULL);
    q->array = malloc(sizeof(int) * size);
    q->alloc_size = size;
    q->head = 0;
    q->tail = 0;
    return q;
}

void free_queue(Queue* queue) {
    pthread_rwlock_destroy(&(queue->q_lock));
    free(queue);
}

int queue_insert(Queue* q, int element) {
    int err = pthread_rwlock_wrlock(&(q->q_lock));
    if (err != 0) {
        return err;
    }
    //int used_size = ((q->tail - q->head) >= 0) ? (q->tail - q->head + 1) : (q->tail - q->head + q->alloc_size + 1);
    //if (used_size == q->alloc_size) {
    if(q->tail - q->head == -1 || q->tail - q->head == q->alloc_size - 1){
        pthread_rwlock_unlock(&(q->q_lock));
        return QUEUE_OVERFLOW;
    } else {
        q->array[q->tail] = element;
        q->tail++;
        if (q->tail == q->alloc_size){
            q->tail = 0;
        }
        else{
            q->tail = q->tail;
        }
    }

    pthread_rwlock_unlock(&(q->q_lock));
    return 0;
}

int queue_remove(Queue* q){
    int err = pthread_rwlock_wrlock(&(q->q_lock));
    if (err != 0) {
        return err;
    }

    if(q->tail == q->head){
        pthread_rwlock_unlock(&(q->q_lock));
        return QUEUE_UNDERFLOW;
    }
    else{
        q->head++;
        if(q->head == q->alloc_size){
            q->head = 0;
        }
        else{
            q->head = q->head;
        }
        pthread_rwlock_unlock(&(q->q_lock));
        return 0;
    }
}

int queue_get_head(Queue * q,int *element){
    int err = pthread_rwlock_rdlock(&(q->q_lock));
    if (err != 0) {
        return err;
    }

    if (q->tail == q->head) {
        pthread_rwlock_unlock(&(q->q_lock));
        return QUEUE_UNDERFLOW;
    }
    else{
        *element = q->array[q->head];
        pthread_rwlock_unlock(&(q->q_lock));
        return 0;
    }
}
