#define _GNU_SOURCE
#include <pthread.h>
struct queue {
    int *array;
    int alloc_size;
    int head;
    int tail;
    pthread_rwlock_t q_lock;
};
typedef struct queue Queue;
#define DEFAULT_SIZE 100
#define QUEUE_OVERFLOW -1
#define QUEUE_UNDERFLOW -2

Queue *init_queue(int size);
void free_queue(Queue *queue);
int queue_insert(Queue *q, int element);
int queue_remove(Queue *q);
int queue_get_head(Queue *q, int *element);
