/* paul/paul_threads.h -- https://github.com/takeiteasy/paul

 Copyright (C) 2025 George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

 /*!
 @header paul_threads.h
 @copyright George Watson GPLv3
 @updated 2025-07-19
 @brief Cross-platform thread pool and job queue implementation for C/C++.
 @discussion
    Implementation is included when PAUL_THREADS_IMPLEMENTATION or PAUL_IMPLEMENTATION is defined.
*/

#ifndef PAUL_THREADS_HEADER
#define PAUL_THREADS_HEADER
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#if __STDC_VERSION__ >= 201112L && !defined(__APPLE__)
#include <threads.h>
#include <stdatomic.h>
#define _PAUL_THREADS_C11
#else
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
typedef CRITICAL_SECTION mtx_t;
typedef CONDITION_VARIABLE cnd_t;
typedef HANDLE thrd_t;
typedef volatile LONG atomic_bool;
typedef volatile LONG atomic_size_t;
#else
#include <pthread.h>
typedef pthread_mutex_t mtx_t;
typedef pthread_cond_t cnd_t;
typedef pthread_t thrd_t;
typedef volatile int atomic_bool;
typedef volatile size_t atomic_size_t;
#endif
#endif

/*!
 @typedef job_t
 @field function Pointer to the function to execute
 @field arg Argument to pass to the function
 @field cleanup Optional cleanup function
 @discussion Job structure for thread pool tasks
*/
typedef struct job {
    void (*function)(void *arg);
    void *arg;
    void (*cleanup)(void *arg); // Optional cleanup function
} job_t;

/*!
 @typedef job_node_t
 @field job The job contained in this node
 @field next Pointer to the next node
 @discussion Node structure for the job queue
*/
typedef struct job_node {
    job_t job;
    struct job_node *next;
} job_node_t;

/*!
 @typedef job_queue_t
 @field head Pointer to the head of the job queue
 @field tail Pointer to the tail of the job queue
 @field mutex Mutex for thread safety
 @field not_empty Condition variable for waiting on empty queue
 @field size Current number of jobs in the queue
 @field max_size Maximum size of the queue
 @field shutdown Flag indicating if the queue is shutting down
 @discussion Job queue structure for thread pool
*/
typedef struct job_queue {
    job_node_t *head;
    job_node_t *tail;
    mtx_t mutex;
    cnd_t not_empty;
    size_t size;
    size_t max_size;
    bool shutdown;
} job_queue_t;

/*!
 @typedef thrd_pool_t
 @field threads Array of worker threads
 @field num_threads Number of worker threads
 @field job_queue The job queue
 @field shutdown Atomic flag indicating if the pool is shutting down
 @field active_threads Atomic count of active threads
 @field pool_mutex Mutex for pool operations
 @field all_threads_idle Condition variable for waiting on all threads idle
 @discussion Thread pool structure
*/
typedef struct {
    thrd_t *threads;
    size_t num_threads;
    job_queue_t job_queue;
    atomic_bool shutdown;
    atomic_size_t active_threads;
    mtx_t pool_mutex;
    cnd_t all_threads_idle;
} thrd_pool_t;

/*!
 @function thrd_pool_create
 @param num_threads Number of worker threads
 @param max_queue_size Maximum size of the job queue
 @return Returns a pointer to the created thread pool, or NULL on failure
 @brief Create a thread pool
*/
thrd_pool_t* thrd_pool_create(size_t num_threads, size_t max_queue_size);
/*!
 @function thrd_pool_submit
 @param pool Pointer to the thread pool
 @param function Pointer to the function to execute
 @param arg Argument to pass to the function
 @param cleanup Optional cleanup function
 @return Returns thrd_success on success, thrd_error on failure
 @brief Submit a job to the thread pool
*/
int thrd_pool_submit(thrd_pool_t *pool, void (*function)(void*), void *arg, void (*cleanup)(void*));
/*!
 @function thrd_pool_wait
 @param pool Pointer to the thread pool
 @brief Wait for all jobs in the thread pool to complete
*/
void thrd_pool_wait(thrd_pool_t *pool);
/*!
 @function thrd_pool_destroy
 @param pool Pointer to the thread pool
 @brief Destroy the thread pool
*/
void thrd_pool_destroy(thrd_pool_t *pool);
/*!
 @function thrd_pool_get_thread_count
 @param pool Pointer to the thread pool
 @return Returns the number of threads in the pool
 @brief Get the number of threads in the thread pool
*/
size_t thrd_pool_get_thread_count(thrd_pool_t *pool);
/*!
 @function thrd_pool_get_active_count
 @param pool Pointer to the thread pool
 @return Returns the number of active threads
 @brief Get the number of active threads in the thread pool
*/
size_t thrd_pool_get_active_count(thrd_pool_t *pool);
/*!
 @function thrd_pool_get_queue_size
 @param pool Pointer to the thread pool
 @return Returns the size of the job queue
 @brief Get the size of the job queue in the thread pool
*/
size_t thrd_pool_get_queue_size(thrd_pool_t *pool);

#ifdef __cplusplus
}
#endif
#endif // PAUL_THREADS_HEADER
#ifndef _PAUL_THREADS_C11
#define thrd_error ((int)-1)
#define thrd_success ((int)0)
#define thrd_busy ((int)1)
#define thrd_nomem ((int)2)
#if defined(_WIN32) || defined(_WIN64)
#define atomic_store(PTR, VAL) InterlockedExchange((PTR), (VAL))
#define atomic_load(PTR) InterlockedCompareExchange((PTR), 0, 0)
#define atomic_fetch_add(PTR, VAL) InterlockedExchangeAdd((PTR), (VAL))
#define atomic_fetch_sub(PTR, VAL) InterlockedExchangeAdd((PTR), -(VAL))

int mtx_init(mtx_t *mtx, int type) {
    if (!mtx)
        return thrd_error;
    InitializeCriticalSection(mtx);
    return thrd_success;
}

int mtx_lock(mtx_t *mtx) {
    if (!mtx)
        return thrd_error;
    EnterCriticalSection(mtx);
    return thrd_success;
}

int mtx_unlock(mtx_t *mtx) {
    if (!mtx)
        return thrd_error;
    LeaveCriticalSection(mtx);
    return thrd_success;
}

int mtx_destroy(mtx_t *mtx) {
    if (!mtx)
        return thrd_error;
    DeleteCriticalSection(mtx);
    return thrd_success;
}

int cnd_init(cnd_t *cond) {
    if (!cond)
        return thrd_error;
    InitializeConditionVariable(cond);
    return thrd_success;
}

int cnd_broadcast(cnd_t *cond) {
    if (!cond)
        return thrd_error;
    WakeAllConditionVariable(cond);
    return thrd_success;
}

int cnd_signal(cnd_t *cond) {
    if (!cond)
        return thrd_error;
    WakeConditionVariable(cond);
    return thrd_success;
}

int cnd_wait(cnd_t *cond, mtx_t *mtx) {
    if (!cond || !mtx)
        return thrd_error;
    SleepConditionVariableCS(cond, mtx, INFINITE);
    return thrd_success;
}

int cnd_destroy(cnd_t *cond) {
    if (!cond)
        return thrd_error;
    // No explicit destruction needed for CONDITION_VARIABLE
    return thrd_success;
}
#else
#define atomic_store(PTR, VAL) __atomic_store_n((PTR), (VAL), __ATOMIC_SEQ_CST)
#define atomic_load(PTR) __atomic_load_n((PTR), __ATOMIC_SEQ_CST)
#define atomic_fetch_add(PTR, VAL) __atomic_fetch_add((PTR), (VAL), __ATOMIC_SEQ_CST)
#define atomic_fetch_sub(PTR, VAL) __atomic_fetch_sub((PTR), (VAL), __ATOMIC_SEQ_CST)

int mtx_init(mtx_t *mtx, int type) {
    pthread_mutexattr_t attr;
    if (!mtx)
        return thrd_error;
    pthread_mutex_init(mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return thrd_success;
}

int mtx_lock(mtx_t *mtx) {
    if (!mtx)
        return thrd_error;
    pthread_mutex_lock(mtx);
    return thrd_success;
}

int mtx_unlock(mtx_t *mtx) {
    if (!mtx)
        return thrd_error;
    pthread_mutex_unlock(mtx);
    return thrd_success;
}

int mtx_destroy(mtx_t *mtx) {
    if (!mtx)
        return thrd_error;
    pthread_mutex_destroy(mtx);
    return thrd_success;
}

int cnd_init(cnd_t *cond) {
    if (!cond)
        return thrd_error;
    pthread_cond_init(cond, NULL);
    return thrd_success;
}

int cnd_broadcast(cnd_t *cond) {
    if (!cond)
        return thrd_error;
    pthread_cond_broadcast(cond);
    return thrd_success;
}

int cnd_signal(cnd_t *cond) {
    if (!cond)
        return thrd_error;
    pthread_cond_signal(cond);
    return thrd_success;
}

int cnd_wait(cnd_t *cond, mtx_t *mtx) {
    if (!cond || !mtx)
        return thrd_error;
    pthread_cond_wait(cond, mtx);
    return thrd_success;
}

int cnd_destroy(cnd_t *cond) {
    if (!cond)
        return thrd_error;
    pthread_cond_destroy(cond);
    return thrd_success;
}

int thrd_create(thrd_t *thr, int (*func)(void*), void *arg) {
    if (!thr || !func)
        return thrd_error;
    if (pthread_create(thr, NULL, (void*(*)(void*))func, arg) != 0)
        return thrd_error;
    return thrd_success;
}

int thrd_join(thrd_t thr, int *res) {
    if (pthread_join(thr, (void**)&res) != 0)
        return thrd_error;
    return thrd_success;
}


#endif
#endif

#if defined(PAUL_THREADS_IMPLEMENTATION) || defined(PAUL_IMPLEMENTATION)
static int job_queue_init(job_queue_t *queue, size_t max_size) {
    if (!queue)
        return thrd_error;
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->max_size = max_size;
    queue->shutdown = false;
    if (mtx_init(&queue->mutex, 0) != thrd_success)
        return thrd_error;
    if (cnd_init(&queue->not_empty) != thrd_success) {
        mtx_destroy(&queue->mutex);
        return thrd_error;
    }
    return thrd_success;
}

static void job_queue_destroy(job_queue_t *queue) {
    if (!queue)
        return;
    mtx_lock(&queue->mutex);
    queue->shutdown = true;
    while (queue->head) {
        job_node_t *temp = queue->head;
        queue->head = queue->head->next;
        if (temp->job.cleanup && temp->job.arg)
            temp->job.cleanup(temp->job.arg);
        free(temp);
    }
    mtx_unlock(&queue->mutex);
    cnd_broadcast(&queue->not_empty);
    mtx_destroy(&queue->mutex);
    cnd_destroy(&queue->not_empty);
}

static int job_queue_enqueue(job_queue_t *queue, job_t job) {
    if (!queue)
        return thrd_error;

    mtx_lock(&queue->mutex);

    if (queue->shutdown) {
        mtx_unlock(&queue->mutex);
        return thrd_error;
    }

    if (queue->max_size > 0 && queue->size >= queue->max_size) {
        mtx_unlock(&queue->mutex);
        return thrd_busy; // Queue is full
    }

    job_node_t *new_node = (job_node_t*)malloc(sizeof(job_node_t));
    if (!new_node) {
        mtx_unlock(&queue->mutex);
        return thrd_nomem;
    }

    new_node->job = job;
    new_node->next = NULL;

    if (queue->tail)
        queue->tail->next = new_node;
    else
        queue->head = new_node;

    queue->tail = new_node;
    queue->size++;
    cnd_signal(&queue->not_empty);
    mtx_unlock(&queue->mutex);
    return thrd_success;
}

static int job_queue_dequeue(job_queue_t *queue, job_t *job) {
    if (!queue || !job)
        return thrd_error;

    mtx_lock(&queue->mutex);

    while (queue->head == NULL && !queue->shutdown)
        cnd_wait(&queue->not_empty, &queue->mutex);

    if (queue->shutdown && queue->head == NULL) {
        mtx_unlock(&queue->mutex);
        return thrd_error;
    }

    job_node_t *node = queue->head;
    *job = node->job;
    queue->head = queue->head->next;

    if (queue->head == NULL)
        queue->tail = NULL;

    queue->size--;
    free(node);
    mtx_unlock(&queue->mutex);
    return thrd_success;
}

static void job_queue_shutdown(job_queue_t *queue) {
    if (!queue)
        return;

    mtx_lock(&queue->mutex);
    queue->shutdown = true;
    mtx_unlock(&queue->mutex);

    cnd_broadcast(&queue->not_empty);
}

static int worker_thread(void *arg) {
    thrd_pool_t *pool = (thrd_pool_t*)arg;
    job_t job;
    while (!atomic_load(&pool->shutdown))
        if (job_queue_dequeue(&pool->job_queue, &job) == thrd_success) {
            // Increment active thread count
            atomic_fetch_add(&pool->active_threads, 1);

            // Execute the job
            if (job.function)
                job.function(job.arg);

            // Decrement active thread count
            atomic_fetch_sub(&pool->active_threads, 1);

            // Signal if all threads are idle
            if (atomic_load(&pool->active_threads) == 0) {
                mtx_lock(&pool->pool_mutex);
                cnd_signal(&pool->all_threads_idle);
                mtx_unlock(&pool->pool_mutex);
            }
        }
    return 0;
}

thrd_pool_t* thrd_pool_create(size_t num_threads, size_t max_queue_size) {
    if (num_threads == 0)
        return NULL;

    thrd_pool_t *pool = (thrd_pool_t*)malloc(sizeof(thrd_pool_t));
    if (!pool)
        return NULL;

    pool->threads = (thrd_t*)malloc(sizeof(thrd_t) * num_threads);
    if (!pool->threads) {
        free(pool);
        return NULL;
    }

    pool->num_threads = num_threads;
    atomic_store(&pool->shutdown, false);
    atomic_store(&pool->active_threads, 0);

    if (job_queue_init(&pool->job_queue, max_queue_size) != thrd_success) {
        free(pool->threads);
        free(pool);
        return NULL;
    }

    if (mtx_init(&pool->pool_mutex, 0) != thrd_success) {
        job_queue_destroy(&pool->job_queue);
        free(pool->threads);
        free(pool);
        return NULL;
    }

    if (cnd_init(&pool->all_threads_idle) != thrd_success) {
        mtx_destroy(&pool->pool_mutex);
        job_queue_destroy(&pool->job_queue);
        free(pool->threads);
        free(pool);
        return NULL;
    }

    // Create worker threads
    for (size_t i = 0; i < num_threads; i++)
        if (thrd_create(&pool->threads[i], worker_thread, pool) != thrd_success) {
            // Cleanup on failure
            atomic_store(&pool->shutdown, true);
            job_queue_shutdown(&pool->job_queue);

            for (size_t j = 0; j < i; j++)
                thrd_join(pool->threads[j], NULL);

            cnd_destroy(&pool->all_threads_idle);
            mtx_destroy(&pool->pool_mutex);
            job_queue_destroy(&pool->job_queue);
            free(pool->threads);
            free(pool);
            return NULL;
        }

    return pool;
}

int thrd_pool_submit(thrd_pool_t *pool, void (*function)(void*), void *arg, void (*cleanup)(void*)) {
    if (!pool || !function)
        return thrd_error;

    if (atomic_load(&pool->shutdown))
        return thrd_error;

    job_t job = {
        .function = function,
        .arg = arg,
        .cleanup = cleanup
    };
    return job_queue_enqueue(&pool->job_queue, job);
}

void thrd_pool_wait(thrd_pool_t *pool) {
    if (!pool)
        return;
    mtx_lock(&pool->pool_mutex);
    while (pool->job_queue.size > 0 || atomic_load(&pool->active_threads) > 0)
        cnd_wait(&pool->all_threads_idle, &pool->pool_mutex);
    mtx_unlock(&pool->pool_mutex);
}

void thrd_pool_destroy(thrd_pool_t *pool) {
    if (!pool)
        return;
    // Signal shutdown
    atomic_store(&pool->shutdown, true);
    job_queue_shutdown(&pool->job_queue);
    // Wait for all threads to finish
    for (size_t i = 0; i < pool->num_threads; i++)
        thrd_join(pool->threads[i], NULL);
    // Cleanup
    job_queue_destroy(&pool->job_queue);
    cnd_destroy(&pool->all_threads_idle);
    mtx_destroy(&pool->pool_mutex);
    free(pool->threads);
    free(pool);
}

size_t thrd_pool_get_thread_count(thrd_pool_t *pool) {
    return pool ? pool->num_threads : 0;
}

size_t thrd_pool_get_active_count(thrd_pool_t *pool) {
    return pool ? atomic_load(&pool->active_threads) : 0;
}

size_t thrd_pool_get_queue_size(thrd_pool_t *pool) {
    if (!pool)
        return 0;
    mtx_lock(&pool->job_queue.mutex);
    size_t size = pool->job_queue.size;
    mtx_unlock(&pool->job_queue.mutex);
    return size;
}
#endif // PAUL_THREADS_IMPLEMENTATION
