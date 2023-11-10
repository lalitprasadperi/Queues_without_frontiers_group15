
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>

#include "queue-wait-free.h"

#define barrier() asm volatile("": : :"memory")

struct wfq *queue;
_Atomic int amount_produced = ATOMIC_VAR_INIT(0);
_Atomic int amount_consumed = ATOMIC_VAR_INIT(0);
_Atomic bool done = ATOMIC_VAR_INIT(false);
_Atomic int retries = ATOMIC_VAR_INIT(0);
_Atomic long long total = ATOMIC_VAR_INIT(0);

#define QSIZE 1000000

#define NUM_THREADS 32

struct item {
  _Atomic int sent, recv;
};

struct item items[NUM_THREADS][QSIZE];

static struct timeval start_time;
static struct timeval end_time;

static void calc_time(struct timeval *start, struct timeval *end) {
    if (end->tv_usec < start->tv_usec) {
        end->tv_sec -= 1;
        end->tv_usec += 1000000;
    }

    assert(end->tv_sec >= start->tv_sec);
    assert(end->tv_usec >= start->tv_usec);
    struct timeval interval = {
        end->tv_sec - start->tv_sec,
        end->tv_usec - start->tv_usec
    };
    printf("%ld.%06ld\t", (long)interval.tv_sec, (long)interval.tv_usec);
}

//-----------------------------------------
//-----------------------------------------
void *enquer_main(void *x)
{
  long tid = (long)x;
  struct timespec start, end;
  for(int i=0;i<QSIZE;i++) {
    //assert(atomic_fetch_add(&items[tid][i].sent, 1) == 0);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    bool r = wfq_enqueue(queue, &items[tid][i]);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    if(r) {
    	atomic_fetch_add(&amount_produced, 1);
    	total += (end.tv_nsec - start.tv_nsec) / 1000;
    } else {
    	items[tid][i].sent = 0;
    	i--;
    	retries++;
    }
  }
  for(int i=0;i<QSIZE;i++) {
    //assert(items[tid][i].sent != 0);
  }
  atomic_thread_fence(memory_order_seq_cst);
  pthread_exit(0);
}

//-----------------------------------------
//-----------------------------------------
void *dequer_main(void *x)
{
  (void)x;
  bool doublechecked = false;
  while(true) {
    void *ret = wfq_dequeue(queue);
    if(ret) {
    	atomic_fetch_add(&amount_consumed, 1);
    	//struct item *it = ret;
    	//assert(atomic_fetch_add(&it->sent, 1) == 1);
    	//assert(atomic_fetch_add(&it->recv, 1) == 0);
    	doublechecked = false;
    } else if(done && doublechecked) {
    	break;
    } else if(done) {
    	doublechecked = true;
    }
  }
  assert(!wfq_dequeue(queue));
  atomic_thread_fence(memory_order_seq_cst);
  //assert(queue->count == 0);
  //assert(queue->head % queue->max == queue->tail);
  pthread_exit(0);
}

//-------------------------------------------------
//-------------------------------------------------
int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;
  int nthr = atoi(argv[1]);
  //int num_producers = NUM_THREADS-1;
  //pthread_t consumers[num_consumers];

  //struct timespec start, end;

   for(int i=0;i<NUM_THREADS;i++) {
        for(int j=0;j<QSIZE;j++) {
    	items[i][j].sent = 0;
    	items[i][j].recv = 0;
    	}
   }

     queue = wfq_create(NULL, 32);
     gettimeofday(&start_time, NULL);
#ifdef TEST1
  int num_producers = nthr;
  pthread_t producers[num_producers];
      for(long i=0;i<num_producers;i++) {
         pthread_create(&producers[i], NULL, enquer_main, (void *)i);
       }

      //-- for(int i=0;i<num_producers;i++) {
      //--    pthread_join(producers[i], NULL);
      //-- }
#endif

#ifdef TEST2
  int num_consumers = nthr;
  pthread_t consumer;
      for(long i=0;i<num_consumers;i++) {
        pthread_create(&consumer, NULL, dequer_main, (void *)i);
       }
        done = true;
    //-- pthread_join(consumer, NULL);
#endif

#ifdef TEST3
  int num_producers = nthr;
  pthread_t producers[num_producers];
      for(long i=0;i<num_producers;i++) {
         pthread_create(&producers[i], NULL, enquer_main, (void *)i);
       }

   atomic_thread_fence(memory_order_seq_cst);
   barrier();

      for(long i=0;i<num_producers;i++) {
         pthread_create(&producers[i], NULL, enquer_main, (void *)i);
       }

  int num_consumers = nthr;
  pthread_t consumer;
      for(long i=0;i<num_consumers;i++) {
        pthread_create(&consumer, NULL, dequer_main, (void *)i);
       }
#endif

   atomic_thread_fence(memory_order_seq_cst);
   gettimeofday(&end_time, NULL);
   calc_time(&start_time, &end_time);

  #ifdef SCAN
   // Scan Queue
    for(int i=0;i<num_producers;i++) {
       for(int j=0;j<QSIZE;j++) {
      	   assert(items[i][j].sent == items[i][j].recv);
      }
    }
   #endif 

}
//-------------------------------------------------


