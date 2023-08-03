#include <pthread.h>
#include <stdlib.h>
#include "queue.h"

struct queue_t {
  int *list;
  int head;
  int tail;
  int size;
  int count;
  pthread_mutex_t lock;
  pthread_cond_t full;
  pthread_cond_t empty;
};

struct queue_t *queue_init(int size)
{
  struct queue_t *q = malloc(sizeof(*q));
  q->list = calloc(size, sizeof(*(q->list)));
  q->head = 0;
  q->tail = 0;
  q->size = size;
  q->count = 0;
  pthread_mutex_init(&q->lock, NULL);
  pthread_cond_init(&q->full, NULL);
  pthread_cond_init(&q->empty, NULL);
  return q;
}

int queue_put(struct queue_t *q, int d)
{
  pthread_mutex_lock(&q->lock);
  
  while (q->count == q->size)
  {
    pthread_cond_wait(&q->full, &q->lock); // sleep till nonfull
  }

  q->list[q->head] = d;
  q->head = (q->head + 1) % q->size;
  q->count++;

  pthread_mutex_unlock(&q->lock);
  pthread_cond_signal(&q->empty);

  return 0;
}

int queue_get(struct queue_t *q)
{
  pthread_mutex_lock(&q->lock);

  while (q->count == 0)
  {
    pthread_cond_wait(&q->empty, &q->lock); // sleep till nonempty
  }

  int r = q->list[q->tail];

  q->tail = (q->tail + 1) % q->size;
  q->count--;

  pthread_mutex_unlock(&q->lock);
  pthread_cond_signal(&q->full);

  return r;
}

void queue_destroy(struct queue_t *q)
{
  pthread_mutex_destroy(&q->lock);
  pthread_cond_destroy(&q->full);
  pthread_cond_destroy(&q->empty);
  free(q->list);
  free(q);
  q = NULL;
}

