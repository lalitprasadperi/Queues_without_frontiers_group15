#include <stdlib.h>
#include <stdbool.h>

typedef struct node_t node_t;
typedef struct queue_t queue_t;
typedef struct pointer_t pointer_t;

struct pointer_t {
    node_t *ptr;
    unsigned int count;
};

struct node_t {
    int value;
    pointer_t next;
};

struct queue_t {
    pointer_t head;
    pointer_t tail;
};

void initialize(queue_t *q) {
    node_t *node;
    node->next.ptr = NULL;
    node->next.count = 0;
    q->head.ptr = q->tail.ptr = node;
    q->head.count = q->tail.count = 0;
}

#define END_MARKER ((void*)-2)

void enqueue(queue_t *q, int value) {
    node_t *node;
    node->value = value;
    node->next.ptr = NULL;
    node->next.count = 0;
    pointer_t tail, next;
    while (1) {
        tail = q->tail;
        pointer_t next = tail.ptr->next;
        if (tail.ptr == q->tail.ptr) {
            if (next.ptr == NULL) {
                if (__sync_val_compare_and_swap((volatile *)&tail.ptr->next, next, node)) {
                    break;
                }
            } else {
                __sync_val_compare_and_swap((volatile *)&q->tail, tail, next);
            }
        }
    }
    __sync_val_compare_and_swap((volatile *)&q->tail, tail, node);
}

bool dequeue(queue_t *q, int *value) {
    pointer_t head, tail, next;
    while (true) {
        head = q->head;
        tail = q->tail;
        next = head.ptr->next;
        if (head.ptr == q->head.ptr) {
            if (head.ptr == tail.ptr) {
                if (next.ptr == NULL) {
                    return false;
                }
                __sync_val_compare_and_swap((volatile *)&q->tail, tail, next);
            } else {
                *value = next.ptr->value;
                if (__sync_val_compare_and_swap((volatile *)&q->head, head, next)) {
                    break;
                }
            }
        }
    }
    head.ptr = NULL;
    return true;
}

