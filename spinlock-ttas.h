#ifndef _SPINLOCK_TTAS_H
#define _SPINLOCK_TTAS_H

typedef struct {
    volatile char lock;
} spinlock;

#define SPINLOCK_ATTR static __inline __attribute__((always_inline, no_instrument_function))

/* Pause instruction to prevent excess processor bus usage */
#define cpu_relax() asm volatile("pause\n": : :"memory")
#define no_op() asm volatile("nop\n": : : )

SPINLOCK_ATTR char __testandtestandset(spinlock *p, int x)
{

/* Test and set a bit */
    char out;

    asm volatile ("lock; bts %2,%1\n"
                  "sbb %0,%0\n"
                  //:"=r" (out), "=m" (*(volatile long long *)ptr)
                  :"=r" (out), "=m" (p->lock)
                  :"Ir" (x)
                  :"memory");

    //printf("TTAS out bit is: %i \n", out);
	return out;
}

SPINLOCK_ATTR void spin_lock(spinlock *lock)
{
   volatile int testbit = 1; // test bit initialize to 1
   volatile char setbit = 1;

    while (setbit==0) {   
    setbit = (__testandtestandset(lock, testbit));  
	// Make memory as set on lock; test for value; lock acquired value test=set 
        //cpu_relax();
        while(setbit==1) {  // backoff by random%10000 nops
		for (int delay_i =0; delay_i < (rand() % 10000) ; delay_i++)
			no_op();
	}
	
    }


    //printf("TTAS test bit is: %i \n", testbit);
     cpu_relax();   
}

SPINLOCK_ATTR void spin_unlock(spinlock *s)
{
    s->lock = 0;  // Unlock is simply to make memory as 0 in test & set and test & test & set spin-locks
}

#define SPINLOCK_INITIALIZER { 0 }

#endif /* _SPINLOCK_TTAS_H */
