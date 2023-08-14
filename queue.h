#pragma once

struct queue_t;

struct queue_t *queue_init(int size);
void queue_destroy(struct queue_t *);
int queue_put(struct queue_t*, int);
int queue_get(struct queue_t*);
