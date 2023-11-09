#include "lock-free-queue.h"
#include <stdlib.h>
#include "spinlock-ttas.h"

int main() {
    spinlock lock = SPINLOCK_INITIALIZER;
    Queue *q = malloc(sizeof(Queue));
    enqueue(q, 1);
    enqueue(q, 2);
    return 0;
}