#include <stddef.h>
#include <stdbool.h>
#include <string.h> /* memcpy */
#include <assert.h>
#include <pthread.h>

#include "spinlock-ttas.h"

#ifdef TTAS
spinlock sl;
#endif

//-----------------------------------------------------------
typedef struct queue_t
{
    void* array;
    size_t capacity;
    size_t sizeof_element;
    size_t in;
    size_t out;
    pthread_mutex_t mutex;
} queue_t;


/* return true on success */
bool queue_init(queue_t* queue, void* array, size_t capacity, size_t sizeof_element);
void queue_destroy(queue_t* queue);
bool queue_front(queue_t * queue, void *dest);
bool queue_back(queue_t * queue, void *dest);
bool queue_enq(queue_t *queue, void const *src);
bool queue_deq(queue_t *queue, void *dest);

/* assert on failure */
size_t queue_size(queue_t * queue);
size_t queue_capacity(queue_t * queue);
bool queue_empty(queue_t * queue);
bool queue_full(queue_t * queue);

//-----------------------------------------------------------
//-----------------------------------------------------------
bool queue_init(queue_t* queue, void* array, size_t capacity, size_t sizeof_element)
{
    bool success;
    
    if ((queue == NULL) || (array == NULL) || (capacity < 2) || (sizeof_element == 0))
    {
        success = false;
    }
    else
    {
        pthread_mutex_init(&queue->mutex, NULL);
        //pthread_mutex_lock(&queue->mutex);
        spin_lock(&sl);
        
        queue->array = array;
        queue->capacity = capacity;
        queue->sizeof_element = sizeof_element;
        queue->in = 0;
        queue->out = 0;
        success = true;
        
        //pthread_mutex_unlock(&queue->mutex);
        spin_unlock(&sl);
    }
    return success;
}

//-----------------------------------------------------------
void queue_destroy(queue_t* queue)
{
   pthread_mutex_destroy(&queue->mutex);
   queue->array = NULL;
   queue->capacity = 0;
   queue->sizeof_element = 0;
   queue->in = 0;
   queue->out = 0;
}

//-----------------------------------------------------------
static void* index_to_address(queue_t const * queue, size_t index)
{
    void* address;
    assert(queue != NULL);
    
    /* No mutex here: this is an internal utility function */
    address = ((unsigned char*)queue->array) + (index * queue->sizeof_element);
    
    return address;
}

//-----------------------------------------------------------
static bool queue_empty_internal(queue_t * queue, bool lock_mutex)
{
    bool empty;
    assert(queue != NULL);
    
    if (lock_mutex) { pthread_mutex_lock(&queue->mutex); }
    empty = (queue->in == queue->out);
    if (lock_mutex) { pthread_mutex_unlock(&queue->mutex); }
    
    return empty;
}

//-----------------------------------------------------------
bool queue_empty(queue_t * queue)
{
    return queue_empty_internal(queue, true);
}

//-----------------------------------------------------------
/* When used internally, mutex might be already locked, so an additional "private" `lock_mutex` argument is needed */
static bool queue_front_internal(queue_t * queue, void *dest, bool lock_mutex)
{
    bool success;
    void *src;
    
    if ((queue == NULL) || (queue_empty_internal(queue, lock_mutex)) || (dest == NULL))
    {
        success = false;
    }
    else
    {
        if (lock_mutex) { pthread_mutex_lock(&queue->mutex); }
        
        src = index_to_address(queue, queue->out);
        memcpy(dest, src, queue->sizeof_element);
        success = true;
        
        if (lock_mutex) { pthread_mutex_unlock(&queue->mutex); }
    }
    return success;
}

//-----------------------------------------------------------
bool queue_front(queue_t * queue, void *dest)
{
    return queue_front_internal(queue, dest, true);
}

//-----------------------------------------------------------
bool queue_back(queue_t * queue, void *dest)
{
    bool success;
    size_t index;
    void *src;
    
    if ((queue == NULL) || (queue_empty(queue)) || (dest == NULL))
    {
        success = false;
    }
    else
    {
        pthread_mutex_lock(&queue->mutex);
        //spin_lock(&sl);
        
        index = (queue->in == 0) ? queue->capacity - 1 : queue->in - 1;
        src = index_to_address(queue, index);
        memcpy(dest, src, queue->sizeof_element);
        success = true;
        
        pthread_mutex_unlock(&queue->mutex);
    }
    return success;
}

//-----------------------------------------------------------
bool queue_enq(queue_t *queue, void const *src)
{
    bool success;
    void *dest;
    
    if ((queue == NULL) || (queue_full(queue)) || (src == NULL))
    {
        success = false;
    }
    else
    {
        spin_lock(&sl);
        
        dest = index_to_address(queue, queue->in);
        memcpy(dest, src, queue->sizeof_element);
        queue->in = (queue->in + 1) % queue->capacity;
        success = true;
        
        spin_unlock(&sl);
    }
    return success;
}

//-----------------------------------------------------------
bool queue_deq(queue_t *queue, void *dest)
{
    bool success;
    
    if ((queue == NULL) || (queue_empty(queue)))
    {
        success = false;
    }
    else
    {
        spin_lock(&sl);
        
        if (dest != NULL)
        {
            queue_front_internal(queue, dest, false);
        }
        queue->out = (queue->out + 1) % queue->capacity;
        success = true;
        
        spin_unlock(&sl);
    }
    return success;
}

//-----------------------------------------------------------
size_t queue_size(queue_t * queue)
{
    size_t size;
    assert(queue != NULL);
    
    //pthread_mutex_lock(&queue->mutex);
    spin_lock(&sl);
    
    if (queue->out <= queue->in)
    {
        size = queue->in - queue->out;
    }
    else
    {
        size = queue->in + queue->capacity - queue->out;
    }
    
    //pthread_mutex_unlock(&queue->mutex);
    spin_unlock(&sl);
    
    return size;
}

//-----------------------------------------------------------
size_t queue_capacity(queue_t * queue)
{
    size_t capacity;
    assert(queue != NULL);
    
    //pthread_mutex_lock(&queue->mutex);
    spin_lock(&sl);
    capacity = queue->capacity - 1;
    //pthread_mutex_unlock(&queue->mutex);
    spin_unlock(&sl);
    
    return capacity;
}

//-----------------------------------------------------------
bool queue_full(queue_t * queue)
{
    bool full;
    assert(queue != NULL);
    
    //pthread_mutex_lock(&queue->mutex);
    spin_lock(&sl);
    full = (((queue->in + 1) % queue->capacity) == queue->out);
    //pthread_mutex_unlock(&queue->mutex);
    spin_unlock(&sl);
    
    return full;
}
//-----------------------------------------------------------
