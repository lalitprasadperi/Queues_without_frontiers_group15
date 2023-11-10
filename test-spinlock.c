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
//#define N_PAIR 16000000
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
static __thread int8_t counter[CACHE_LINE*NCOUNTER];


#ifdef MCS
typedef struct mcs_lock_t mcs_lock_t;

struct mcs_lock_t
{
	mcs_lock_t *next;
	int spin;
};

typedef struct mcs_lock_t *mcs_lock;

mcs_lock cnt_lock = NULL;
//mcs_lock msl;

static void lock_mcs(mcs_lock *m, mcs_lock_t *me)
{
	mcs_lock_t *tail;
	
	me->next = NULL;
	me->spin = 0;

	tail = xchg_64(m, me);
	
	if (!tail) return;
	  tail->next = me;

	barrier();
	
	/* Spin on my spin variable */
	while (!me->spin) cpu_relax();
	
	return;
}

static void unlock_mcs(mcs_lock *m, mcs_lock_t *me)
{
	// Wait for successor
	if (!me->next)
	{
		// Try to atomically unlock
		if (cmpxchg(m, me, NULL) == me) return;

		// Wait for successor to appear
		while (!me->next) cpu_relax();
	}
	// Unlock next
	me->next->spin = 1;	
}

#elif CLH

typedef struct clh_lock_t clh_lock_t;

struct clh_lock_t
{
	clh_lock_t *next;
	int spin;
};

typedef struct clh_lock_t *clh_lock;

clh_lock cnt_lock = NULL;

static void lock_clh(clh_lock *m, clh_lock_t *me)
{
	clh_lock_t *tail;
	
	me->next = NULL;
	me->spin = 0;

	tail = xchg_64(m, me);
	
	/* No one there? */
	if (!tail) return;

	/* Someone there, need to link in */
	tail->next = me;

	/* Make sure we do the above setting of next. */
	barrier();
	
	/* Spin on my spin variable */
	while (!me->spin) cpu_relax();

	return;
}

static void unlock_clh(clh_lock *m, clh_lock_t *me)
{
	/* No successor yet? */
	if (!me->next)
	{
		/* Try to atomically unlock */
		if (cmpxchg(m, me, NULL) == me) return;
	
		/* Wait for successor to appear */
		while (!me->next) cpu_relax();
	}

	/* Unlock next one */
	me->next->spin = 1;	
}


#elif ONERING
linuxstylelock linuxlock_t;

#else
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

void *inc_thread(void *id) {
    int n = N_PAIR / nthr;
    assert(n * nthr == N_PAIR);

#ifdef MCS
    mcs_lock_t local_lock;
#endif

#ifdef CLH
    clh_lock_t local_lock;
#endif

#ifdef BIND_CORE
    bind_core((int)(long)(id));
#endif

    wait_flag(&wflag, nthr);

    if (((long) id == 0)) {
        /*printf("get start time\n");*/
        gettimeofday(&start_time, NULL);
    }

    /* Start lock unlock test. */
    for (int i = 0; i < n; i++) {

#ifdef MCS
        lock_mcs(&cnt_lock, &local_lock);
        //lock_mcs(&msl);
        for (int j = 0; j < NCOUNTER; j++) counter[j*CACHE_LINE]++;
        unlock_mcs(&cnt_lock, &local_lock);
        //unlock_mcs(&msl);

#elif CLH
        lock_clh(&cnt_lock, &local_lock);
        for (int j = 0; j < NCOUNTER; j++) counter[j*CACHE_LINE]++;
        unlock_clh(&cnt_lock, &local_lock);
	
#elif ONERING
        spin_lock(&linuxlock_t);
        for (int j = 0; j < NCOUNTER; j++) counter[j*CACHE_LINE]++;
        spin_unlock(&linuxlock_t);

#elif RTM
        int status;
        if ((status = _xbegin()) == _XBEGIN_STARTED) {
            for (int j = 0; j < NCOUNTER; j++) counter[j*CACHE_LINE]++;
            if (sl == BUSY)
                _xabort(1);
            _xend();
        } else {
            spin_lock(&sl);
            for (int j = 0; j < NCOUNTER; j++) counter[j*CACHE_LINE]++;
            spin_unlock(&sl);
        }
#else
        spin_lock(&sl);
        for (int j = 0; j < NCOUNTER; j++) counter[j*CACHE_LINE]++;
        spin_unlock(&sl);
#endif
    }

    if (__sync_fetch_and_add((uint32_t *)&wflag, -1) == 1) {
        /*printf("get end time\n");*/
        gettimeofday(&end_time, NULL);
    }
    return NULL;
}


int main(int argc, const char *argv[])
{
    pthread_t *thr;
    int ret = 0;

    if (argc != 2) {
        printf("Usage: %s <num of threads>\n", argv[0]);
        exit(1);
    }

    nthr = atoi(argv[1]);
    /*printf("using %d threads\n", nthr);*/
    thr = calloc(sizeof(*thr), nthr);

    // Start thread
    for (long i = 0; i < nthr; i++) {
        if (pthread_create(&thr[i], NULL, inc_thread, (void *)i) != 0) {
            perror("thread creating failed");
        }
    }
    // join thread
    for (long i = 0; i < nthr; i++)
        pthread_join(thr[i], NULL);

    calc_time(&start_time, &end_time);

    /*
     *for (int i = 0; i < NCOUNTER; i++) {
     *    if (counter[i] == N_PAIR) {
     *    } else {
     *        printf("counter %d error\n", i);
     *        ret = 1;
     *    }
     *}
     */

    return ret;
}
