#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H
#include <stdlib.h>

typedef struct {
    int data;
    struct QNode *next;
} QNode;

typedef struct {
    struct QNode *head;
    struct QNode *tail;
} Queue;

#define QUEUE_ATTR static __inline __attribute__((always_inline, no_instrument_function))

void enqueue(Queue * Q, int data) {
    struct QNode *new_node = malloc(sizeof(struct QNode));
    new_node->data = data;
    new_node->next = NULL;


    // struct QNode *new_node = malloc(sizeof(struct QNode));
    // new_node->data = data;
    // new_node->next = NULL;

    // while (true) {
    //     struct QNode *tail = Q.tail;
    //     struct QNode *next = tail->next;

    //     if (tail == Q.tail) {
    //         if (next == NULL) {
    //             if (__sync_bool_compare_and_swap(&tail->next, NULL, new_node)) {
    //                 __sync_bool_compare_and_swap(&Q.tail, tail, new_node);
    //                 return;
    //             }
    //         } else {
    //             __sync_bool_compare_and_swap(&Q.tail, tail, next);
    //         }
    //     }
    // }
}

int dequeue(Queue * Q, int *data) {
    while (true) {
        struct QNode *head = Q.head;
        struct QNode *tail = Q.tail;
        struct QNode *next = head->next;

        if (head == Q.head) {
            if (head == tail) {
                if (next == NULL) {
                    return 0;
                }
                __sync_bool_compare_and_swap(&Q.tail, tail, next);
            } else {
                *data = next->data;
                if (__sync_bool_compare_and_swap(&Q.head, head, next)) {
                    free(head);
                    return 1;
                }
            }
        }
    }
}

#endif

