#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "delay.h"
#include "queue.h"

#ifndef LOGN_OPS
#define LOGN_OPS 7
#endif

static long nops;
static queue_t * q;
static handle_t ** hds;

void init(int nprocs, int logn) {
  /** Use 10^7 as default input size. */
  if (logn == 0) logn = LOGN_OPS;

  /** Compute the number of ops to perform. */
  nops = 1;
  int i;
  for (i = 0; i < logn; ++i) {
    nops *= 10;
  }

  printf("  Number of operations: %ld\n", nops);

  // FIXME: sizeof(queue_t) varies, allocate 4MB
  q = align_malloc(PAGE_SIZE, 4194304);
  queue_init(q, nprocs);

  hds = align_malloc(PAGE_SIZE, sizeof(handle_t * [nprocs]));
}

void thread_init(int id, int nprocs) {
  hds[id] = align_malloc(PAGE_SIZE, sizeof(handle_t));
  queue_register(q, hds[id], id);
}

void thread_exit(int id, int nprocs) {
  queue_free(q, hds[id]);
}

void * benchmark(int id, int nprocs) {
  void * val = (void *) (intptr_t) (id + 1);
  handle_t * th = hds[id];

  delay_t state;
  delay_init(&state, id);

  struct drand48_data rstate;
  srand48_r(id, &rstate);

  int i;
  int runs = 1000000;
  for (i = 0; i < runs; ++i) {
    long n;
#ifdef TEST1
    enqueue(q, th, val);
#endif
#ifdef TEST2
    dequeue(q, th);
#endif
#ifdef TEST3
    if (i < runs / 2) {
      enqueue(q, th, val);
    } else {
      dequeue(q, th);
    }
#endif
//    delay_exec(&state);
  }

  return val;
}

int verify(int nprocs, void ** results) {
  return 0;
}
