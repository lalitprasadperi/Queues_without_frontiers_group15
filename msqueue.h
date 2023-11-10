#include "hzdptr.h"
#include <stdbool.h>
#include "align.h"

#define EMPTY (void *) -1

typedef struct _node_t {
  struct _node_t * volatile next;
  void * data;
} node_t;

typedef struct _queue_t {
  struct _node_t * volatile head;
  struct _node_t * volatile tail;
  int nprocs;
} queue_t;

typedef struct _handle_t {
  hzdptr_t hzd;
} handle_t;

void queue_init(queue_t * q, int nprocs);
void queue_register(queue_t * q, handle_t * th, int id);
void enqueue(queue_t * q, handle_t * th, void * v);
bool dequeue(queue_t * q, handle_t * th);