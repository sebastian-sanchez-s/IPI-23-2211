#include "common.h"
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
  struct queue_t *q;
  MALLOC(q, sizeof(struct queue_t));
  MALLOC(q->list, sizeof(int[size+1]));
  q->head = 0;
  q->tail = 0;
  q->size = size+1;
  q->count = 0;
  pthread_mutex_init(&q->lock, NULL);
  pthread_cond_init(&q->full, NULL);
  pthread_cond_init(&q->empty, NULL);
  return q;
}

int queue_put(struct queue_t *q, int d)
{
  PANIKON(q==NULL, "queue is null");
  pthread_mutex_lock(&q->lock);
  
  while (q->count == q->size)
  {
    pthread_cond_wait(&q->full, &q->lock); // sleep till nonfull
  }

  q->list[q->head] = d;
  q->head = (q->head + 1) % q->size;
  q->count += 1;

  pthread_cond_signal(&q->empty);
  pthread_mutex_unlock(&q->lock);

  return 0;
}

int queue_get(struct queue_t *q)
{
  PANIKON(q==NULL, "queue is null");
  pthread_mutex_lock(&q->lock);

  while (q->count == 0)
  {
    pthread_cond_wait(&q->empty, &q->lock); // sleep till nonempty
  }

  int r = q->list[q->tail];

  q->tail = (q->tail + 1) % q->size;
  q->count -= 1;

  pthread_cond_signal(&q->full);
  pthread_mutex_unlock(&q->lock);

  return r;
}

void queue_destroy(struct queue_t *q)
{
  PANIKON(q==NULL, "queue is null");
  pthread_mutex_destroy(&q->lock);
  pthread_cond_destroy(&q->full);
  pthread_cond_destroy(&q->empty);
  free(q->list);
  free(q);
}

