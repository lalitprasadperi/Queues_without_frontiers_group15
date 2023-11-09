#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <errno.h>

#include "spinlock-ttas.h"

#ifndef cpu_relax
#define cpu_relax() asm volatile("pause\n": : :"memory")
#endif

/*
 * You need  to provide your own code for bidning threads to processors
 * Use lscpu on rlogin servers to get an idea of the number and topology
 * of processors. The bind_
 */

/* Number of total lock/unlock pair.
 * Note we need to ensure the total pair of lock and unlock opeartion are the
 * same no matter how many threads are used. */
#define N_PAIR 16000000

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

#ifdef TTAS
spinlock sl;
#endif

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


//---------------------------------------------------------

#define SIZE 1000000

//struct QNode {
//  uint32_t queue_data;
//  uint8_t  thread_id;
//}

typedef struct {
  uint32_t data;
  uint8_t  thread_id;
  int *current;
  int *next;
} Qnode;

int tail = 0;
int head = 1;

Qnode qn;

//void enqueue (Q, data) //enqueue a node with content "data" at the tail of the queue pointed to by Q
void enqueue () //enqueue a node with content "data" at the tail of the queue pointed to by Q
{
        spin_lock(&sl);
        //for (int j = 0; j < NCOUNTER; j++) counter[j*CACHE_LINE]++;
        spin_unlock(&sl);

}

//int dequeue(Q) //dequeue a node from the head of the queue pointed by Q and return its data
void dequeue() //dequeue a node from the head of the queue pointed by Q and return its data
{
        spin_lock(&sl);
        //for (int j = 0; j < NCOUNTER; j++) counter[j*CACHE_LINE]++;
        spin_unlock(&sl);
}

void scan()
{
        spin_lock(&sl);
        //for (int j = 0; j < NCOUNTER; j++) counter[j*CACHE_LINE]++;
        spin_unlock(&sl);

}

//------------------------------------------------
// Thread creation here
//------------------------------------------------
void *inc_thread(void *id) {

    int n = N_PAIR / nthr;
    assert(n * nthr == N_PAIR);

    int num_enq = SIZE / nthr;
    int num_deq = SIZE / nthr;

#ifdef BIND_CORE
    bind_core((int)(long)(id));
#endif

    wait_flag(&wflag, nthr);

     if (((long) id == 0)) {
        /*printf("get start time\n");*/
        gettimeofday(&start_time, NULL);
     }

    /* Start enque / deque / scan test. */

#ifdef ENQ
    for (int i = 0; i < num_enq ; i++) {
        enqueue();
    }
#endif

#ifdef DEQ
    for (int i = 0; i < num_deq ; i++) {
        dequeue();
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

     gettimeofday(&end_time, NULL);
     calc_time(&start_time, &end_time);

     return ret;
}


//---------------------------------------------------------
//-- 
//-- main()
//-- {
//--     int ch;
//--     while (1)
//--     {
//--         printf("1.Enqueue Operation\n");
//--         printf("2.Dequeue Operation\n");
//--         printf("3.Display the Queue\n");
//--         printf("4.Exit\n");
//--         printf("Enter your choice of operations : ");
//--         scanf("%d", &ch);
//--         switch (ch)
//--         {
//--             case 1:
//--             enqueue();
//--             break;
//--             case 2:
//--             dequeue();
//--             break;
//--             case 3:
//--             show();
//--             break;
//--             case 4:
//--             exit(0);
//--             default:
//--             printf("Incorrect choice \n");
//--         } 
//--     } 
//-- } 
//--  
//-- void enqueue()
//-- {
//--     int insert_item;
//--     if (Rear == SIZE - 1)
//--        printf("Overflow \n");
//--     else
//--     {
//--         if (Front == - 1)
//--       
//--         Front = 0;
//--         printf("Element to be inserted in the Queue\n : ");
//--         scanf("%d", &insert_item);
//--         Rear = Rear + 1;
//--         inp_arr[Rear] = insert_item;
//--     }
//-- } 
//--  
//-- void dequeue()
//-- {
//--     if (Front == - 1 || Front > Rear)
//--     {
//--         printf("Underflow \n");
//--         return ;
//--     }
//--     else
//--     {
//--         printf("Element deleted from the Queue: %d\n", inp_arr[Front]);
//--         Front = Front + 1;
//--     }
//-- } 
//--  
//-- void show()
//-- {
//--     
//--     if (Front == - 1)
//--         printf("Empty Queue \n");
//--     else
//--     {
//--         printf("Queue: \n");
//--         for (int i = Front; i <= Rear; i++)
//--             printf("%d ", inp_arr[i]);
//--         printf("\n");
//--     }
//-- }
//-- 
//-- 
