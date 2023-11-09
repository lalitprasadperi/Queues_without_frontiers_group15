#include "lock-free-queue.h"
#include <stdlib.h>
#include "spinlock-ttas.h"

int main() {
    queue_t *q;
    initialize(q);
    enqueue(q, 1);
    enqueue(q, 2);
    enqueue(q, 3);
}