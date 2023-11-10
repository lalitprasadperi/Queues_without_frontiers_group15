CFLAGS = -O2 -g -std=gnu99 -Wall -march=x86-64 -Wextra -Wunused-variable
LDFLAGS = -lpthread

#programs = test-spinlock-ttas  test-enque-ttas test-deque-ttas test-enq-deq-ttas test-enque-lock-free test-deque-lock-free test-enq-deq-lock-free test-enque-wait-free test-deque-wait-free test-enq-deq-wait-free
programs = test-spinlock-ttas test-enque-ttas test-deque-ttas test-enq-deq-ttas test-enque-wait-free test-deque-wait-free test-enq-deq-wait-free 

all: $(programs)

test-spinlock-ttas: test-spinlock.c
	$(CC) $(CFLAGS) -DTTAS $^ -o $@ $(LDFLAGS)

test-enque-ttas: test-queue-ttas.c
	$(CC) $(CFLAGS) -DTTAS -DENQ -DTEST1 $^ -o $@ $(LDFLAGS)
test-deque-ttas: test-queue-ttas.c
	$(CC) $(CFLAGS) -DTTAS -DDEQ -DTEST2 $^ -o $@ $(LDFLAGS)
test-enq-deq-ttas: test-queue-ttas.c
	$(CC) $(CFLAGS) -DTTAS -DENQ -DDEQ -DTEST3 $^ -o $@ $(LDFLAGS)

#test-enque-lock-free: test-queue-lock-free.c
#	$(CC) $(CFLAGS) -DENQ $^ -o $@ $(LDFLAGS)
#test-deque-lock-free: test-queue-lock-free.c
#	$(CC) $(CFLAGS) -DDEQ $^ -o $@ $(LDFLAGS)
#test-enq-deq-lock-free: test-queue-lock-free.c
#	$(CC) $(CFLAGS) -DENQ -DDEQ $^ -o $@ $(LDFLAGS)

test-enque-wait-free: test-queue-wait-free.c queue-wait-free.c
	$(CC) $(CFLAGS) -DTTAS -DENQ -DTEST1 $^ -o $@ $(LDFLAGS)

test-deque-wait-free: test-queue-wait-free.c queue-wait-free.c
	$(CC) $(CFLAGS) -DTTAS -DDEQ -DTEST2 $^ -o $@ $(LDFLAGS)

test-enq-deq-wait-free: test-queue-wait-free.c queue-wait-free.c
	$(CC) $(CFLAGS) -DTTAS -DENQ -DDEQ -DTEST3 $^ -o $@ $(LDFLAGS)

%:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o
	-rm -f $(programs)


