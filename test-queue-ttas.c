#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "queue-ttas.h"

#ifndef cpu_relax
#define cpu_relax() asm volatile("pause\n": : :"memory")
#endif

#define barrier() asm volatile("": : :"memory")

/*
 * You need  to provide your own code for bidning threads to processors
 * Use lscpu on rlogin servers to get an idea of the number and topology
 * of processors. The bind_
 */

/* Number of total lock/unlock pair.
 * Note we need to ensure the total pair of lock and unlock opeartion are the
 * same no matter how many threads are used. */
#define N_PAIR 1000000

/* Bind threads to specific cores. The goal is to make threads locate on the
 * same physical CPU. Modify bind_core before using this. */
//#define BIND_CORE

static int nthr = 0;

static volatile uint32_t wflag;
/* Wait on a flag to make all threads start almost at the same time. */
void wait_flag(volatile uint32_t *flag, uint32_t expect) {
    __sync_fetch_and_add((uint32_t *)flag, 1);
    while (*flag != expect) {
        cpu_relax();
    }
}

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

// Use an array of counter to see effect on RTM if touches more cache line.
#define NCOUNTER 1
#define CACHE_LINE 64

// Use thread local counter to avoid cache contention between cores.
// For TSX, this avoids TX conflicts so the performance overhead/improvement is
// due to TSX mechanism.
//-- static __thread int8_t counter[CACHE_LINE*NCOUNTER];

//--#ifdef TTAS
//--spinlock sl;
//--#endif

#ifdef BIND_CORE
void bind_core(int threadid) {
    /* cores with logical id 2x   are on node 0 */
    /* cores with logical id 2x+1 are on node 1 */
    /* each node has 16 cores, 32 hyper-threads */
    int phys_id = threadid / 16;
    int core = threadid % 16;

    int logical_id = 2 * core + phys_id;
    /*printf("thread %d bind to logical core %d on physical id %d\n", threadid, logical_id, phys_id);*/

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(logical_id, &set);

    if (sched_setaffinity(0, sizeof(set), &set) != 0) {
        perror("Set affinity failed");
        exit(EXIT_FAILURE);
    }
}
#endif

#define QSIZE 1000000

//-- typedef struct {
//--   uint32_t data;
//--   uint8_t  thread_id;
//--   int *current;
//--   int *next;
//-- } Qnode;
//-- 
//-- int tail = 0;
//-- int head = 1;
//-- 
//-- //Qnode qn;
//---------------------------------------------------------

int *head;
int *tail;

typedef struct Qnode
{
  uint32_t data;
} Qnode;

Qnode Qnode_array[QSIZE];

queue_t queue;

//---------------------------------------------------------
// Scan through the queue to check for correctness
void scan()
{
      //   spin_lock(&sl);
      //  //for (int j = 0; j < NCOUNTER; j++) counter[j*CACHE_LINE]++;
      //  spin_unlock(&sl);
}

//------------------------------------------------
// Thread creation here
//------------------------------------------------
void *inc_thread(void *id) {


#ifdef BIND_CORE
    bind_core((int)(long)(id));
#endif

    wait_flag(&wflag, nthr);

     if (((long) id == 0)) {
        /*printf("get start time\n");*/
        gettimeofday(&start_time, NULL);
     }

    /* Start enque / deque / scan test. */
#ifdef TEST1
    int num_enq = QSIZE / nthr;
    for (int i = 0; i < num_enq ; i++) {
      queue_enq(&queue,&head); 
    }
#endif

#ifdef TEST2
    int num_deq = QSIZE / nthr;
    for (int i = 0; i < num_deq ; i++) {
      queue_deq(&queue,&tail); 
    }
#endif

#ifdef TEST3
    int num_enq = QSIZE / nthr;
    int num_deq = QSIZE / nthr;
    for (int i = 0; i < num_enq ; i++) {
      queue_enq(&queue,&head); 
    }
    barrier();   // barrier after first enqueue
    atomic_thread_fence(memory_order_relaxed);

    for (int i = 0; i < num_enq ; i++) {
      queue_enq(&queue,&head); 
    }

    for (int i = 0; i < num_deq ; i++) {
      queue_deq(&queue,&tail); 
    }
#endif

#ifdef SCAN
    for (int i = 0; i < num_enq ; i++) {
        scan();
    }
#endif

    if (__sync_fetch_and_add((uint32_t *)&wflag, -1) == 1) {
        gettimeofday(&end_time, NULL);
    }
    return NULL;
}

//-----------------------------------
// MAIN
//-----------------------------------
int main(int argc, const char *argv[])
{

    pthread_t *thr;
    int ret = 0;

    if (argc != 2) {
        printf("Usage: %s <num of threads>\n", argv[0]);
        exit(1);
    }

     // Initialize Array based Q
     queue_init(&queue, Qnode_array, QSIZE,sizeof(Qnode));

     gettimeofday(&start_time, NULL);

    // Main body-here
    // -------------------------------------
     nthr = atoi(argv[1]);
     /*printf("using %d threads\n", nthr);*/
     thr = calloc(sizeof(*thr), nthr);
 
     // Start thread
     for (long i = 0; i < nthr; i++) {
         if (pthread_create(&thr[i], NULL, inc_thread, (void *)i) != 0) {
             perror("thread creation failed");
         }
     }
     // join thread
     for (long i = 0; i < nthr; i++)
         pthread_join(thr[i], NULL);
    // -------------------------------------
    //
    //printf("%d thr PASS: ",nthr);

     gettimeofday(&end_time, NULL);
     calc_time(&start_time, &end_time);

     // De-alloc Queue
     queue_destroy(&queue);

     return ret;
}

//---------------------------------------------------------

