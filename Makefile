TESTS = msqueue

# if using clang, please also specify -mcx16 for x86-64
CC = gcc
CFLAGS = -g -Wall -O3 -pthread -D_GNU_SOURCE
LDLIBS = -lpthread -lm

ifeq (${VERIFY}, 1)
	CFLAGS += -DVERIFY
endif

ifeq (${SANITIZE}, 1)
	CFLAGS += -fsanitize=address -fno-omit-frame-pointer
	LDLIBS += -lasan
	LDFLAGS = -fsanitize=address
endif

all: $(TESTS)


msqueue: CFLAGS += -DMSQUEUE -DTEST1

$(TESTS): harness.o
$(TESTS): tests.o

msqueue lcrq: hzdptr.o

clean:
	rm -f $(TESTS) *.o


