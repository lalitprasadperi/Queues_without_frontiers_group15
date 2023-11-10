
#ifndef __WFQ_H
#define __WFQ_H

#include <stdint.h>
#ifndef __cplusplus
#include <stdatomic.h>
#endif
#include <stdbool.h>
#include <sys/types.h>

#define WFQ_MALLOC 1

#ifndef __cplusplus
struct wfq {
	_Atomic size_t count;
	_Atomic size_t head;
	size_t tail;
	size_t max;
	void * _Atomic *buffer;
	int flags;
};
#else
struct wfq;
#endif

#ifdef __cplusplus
extern "C" {
#endif
/* create a new wfq. If n == NULL, it will allocate
 * a new one and return it. If n != NULL, it will
 * initialize the structure that was passed in. 
 * capacity must be greater than 1, and it is recommended
 * to be much, much larger than that. It must also be a power of 2. */
struct wfq *wfq_create(struct wfq *n, size_t capacity);

/* enqueue an item into the queue. Returns true on success
 * and false on failure (queue full). This is safe to call
 * from multiple threads */
bool wfq_enqueue(struct wfq *q, void *obj);

/* dequeue an item from the queue and return it.
 * THIS IS NOT SAFE TO CALL FROM MULTIPLE THREADS.
 * Returns NULL on failure, and the item it dequeued
 * on success */
void *wfq_dequeue(struct wfq *q);

/* get the number of items in the queue currently */
size_t wfq_count(struct wfq *q);

/* get the capacity of the queue */
size_t wfq_capacity(struct wfq *q);

/* destroy a wfq. Frees the internal buffer, and
 * frees q if it was created by passing NULL to wfq_create */
void wfq_destroy(struct wfq *q);

#ifdef __cplusplus
}
#endif

#endif

