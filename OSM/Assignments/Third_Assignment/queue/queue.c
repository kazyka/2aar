#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "queue.h"

#define INIT_SIZE       8
#define GROWTH_FACTOR   2
#define SHRINK_FACTOR   4

// All of the below functions assume that q is not NULL.

pthread_mutex_t lock;
pthread_cond_t cond;

static inline size_t
parent(size_t i) {
  return i / 2;
}

static inline size_t
left(size_t i) {
  return i == 0 ? 1 : i * 2;
}

static inline size_t
right(size_t i) {
  return i == 0 ? 2 : i * 2 + 1;
}

static inline void
exchange(struct node *x, struct node *y) {
  struct node tmp_node = *x;
  *x = *y;
  *y = tmp_node;
}

static void
queue_heap_up(struct node *array, size_t i) {
  while (i > 0 && array[parent(i)].pri < array[i].pri) {
    exchange(&array[parent(i)], &array[i]);
    i = parent(i);
  }
}

static void
queue_heap_down(struct node *array, size_t count, size_t i) {
  size_t max_child;

  while (left(i) < count) {
    max_child = left(i);
    if (right(i) < count && array[right(i)].pri > array[left(i)].pri) {
      max_child = right(i);
    }
    if (array[max_child].pri > array[i].pri) {
      exchange(&array[max_child], &array[i]);
    }
    i = max_child;
  }
}

int
queue_init(struct queue *q) {
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&cond, NULL);

  q->root = (struct node *)malloc(
    sizeof(struct node) * INIT_SIZE);
  q->next = q->root;
  q->count = 0;
  q->size = INIT_SIZE;

  pthread_mutex_unlock(&lock);

  return 0;
}

static int
queue_grow(struct queue *q) {
  size_t new_size;
  struct node *new_root;

  new_size = q->size * GROWTH_FACTOR;
  new_root = (struct node *)realloc(q->root,
    sizeof(struct node) * new_size);

  if (new_root == NULL) return QUEUE_OVERFLOW;

  q->size = new_size;
  q->root = new_root;
  q->next = q->root + q->count;

  return 0;
}


int
queue_push(struct queue *q, int pri) {
  pthread_mutex_lock(&lock);
  int retval;

  if (q->count >= q->size) {
    retval = queue_grow(q);
    if (retval != 0) 
      {
        return retval;
      }
  }

  q->next->pri = pri;
  q->next++;

  queue_heap_up(q->root, q->count);
  q->count++;


  pthread_mutex_unlock(&lock);
  pthread_cond_signal(&cond);
  return 0;
}

int
queue_pop(struct queue *q, int *pri_ptr) {
  pthread_mutex_lock(&lock);

  while (q->count == 0) 
  {
    pthread_cond_wait(&cond, &lock);
  }
  pthread_mutex_unlock(&lock);
  *pri_ptr = q->root->pri;

  q->count--;
  q->next--;

  exchange(q->root, q->next);

  queue_heap_down(q->root, q->count, 0);
  return 0;
}

int queue_destroy(struct queue *q) {
  free(q->root);
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&cond);
  return 0;
}
