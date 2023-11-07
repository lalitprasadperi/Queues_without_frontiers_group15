CFLAGS = -O2 -g -std=gnu99 -Wall -march=x86-64
LDFLAGS = -lpthread


programs = test-spinlock-ttas 

all: $(programs)

test-spinlock-ttas: test-spinlock.c
	$(CC) $(CFLAGS) -DTTAS $^ -o $@ $(LDFLAGS)

%:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o
	-rm -f $(programs)
