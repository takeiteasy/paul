/* jeff/events.c -- https://github.com/takeiteasy/jeff

 Copyright (C) 2024  George Watson

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

#define PTHREAD_SHIM_IMPLEMENTATION
#include "jeff.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>

void thread_sleep(const struct timespec *timeout) {
    Sleep(timespec2ms(timeout));
}

void thread_yield(void) {
    SwitchToThread();
}
#else
#include <unistd.h>

void thread_sleep(const struct timespec *timeout) {
    nanosleep(timeout, NULL);
}

void thread_yield(void) {
    sched_yield();
}
#endif

struct timespec thread_timeout(unsigned int milliseconds) {
    return (struct timespec) {
        .tv_sec = (milliseconds / 1000) + time(NULL),
        .tv_nsec = (milliseconds % 1000) * 1000000
    };
}

static void* thread_pool_worker(void *arg) {
    thread_pool_t *pool = (thread_pool_t*)arg;
    while (!pool->kill) {
        while (!pool->head && !pool->kill)
            pthread_cond_wait(&pool->workCond, &pool->workMutex);
        if (pool->kill)
            break;

        thread_work_t *work = pool->head;
        if (!work)
            break;
        if (!work->next) {
            pool->head = NULL;
            pool->tail = NULL;
        } else
            pool->head = work->next;
        pool->workingCount++;

        pthread_mutex_unlock(&pool->workMutex);
        work->func(work->arg);
        free(work);
        pthread_mutex_lock(&pool->workMutex);
        if (!pool->kill && !--pool->workingCount && pool->head)
            pthread_cond_signal(&pool->workingCond);
        pthread_mutex_unlock(&pool->workMutex);
    }

    pool->threadCount--;
    pthread_cond_signal(&pool->workingCond);
    pthread_mutex_unlock(&pool->workMutex);
    return NULL;
}

thread_pool_t* thread_pool(size_t maxThreads) {
    if (!maxThreads)
        return NULL;
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));
    pthread_mutex_init(&pool->workMutex, NULL);
    pthread_cond_init(&pool->workCond, NULL);
    pthread_cond_init(&pool->workingCond, NULL);
    pool->head = pool->tail = NULL;
    pool->threadCount = maxThreads;

    pthread_t pthread_;
    for (int i = 0; i < maxThreads; i++) {
        pthread_create(&pthread_, NULL, thread_pool_worker, (void*)pool);
        pthread_detach(pthread_);
    }
    return pool;
}

void thread_pool_destroy(thread_pool_t *pool) {
    pthread_mutex_lock(&pool->workMutex);
    thread_work_t *work = pool->head;
    while (work) {
        thread_work_t *tmp = work->next;
        free(work);
        work = tmp;
    }
    pthread_mutex_unlock(&pool->workMutex);
    thread_pool_join(pool);
    pthread_mutex_destroy(&pool->workMutex);
    pthread_cond_destroy(&pool->workCond);
    pthread_cond_destroy(&pool->workingCond);
    free(pool);
}

int thread_pool_push_work(thread_pool_t *pool, void(*func)(void*), void *arg) {
    thread_work_t *work = malloc(sizeof(thread_work_t));
    work->arg = arg;
    work->func = func;
    work->next = NULL;
    pthread_mutex_lock(&pool->workMutex);
    if (!pool->head) {
        pool->head = work;
        pool->tail = pool->head;
    } else {
        pool->tail->next = work;
        pool->tail = work;
    }
    pthread_cond_broadcast(&pool->workCond);
    pthread_mutex_unlock(&pool->workMutex);
    return 1;
}

int thread_pool_push_work_priority(thread_pool_t *pool, void(*func)(void*), void *arg) {
     thread_work_t *work = malloc(sizeof(thread_work_t));
    work->arg = arg;
    work->func = func;
    work->next = NULL;
    pthread_mutex_lock(&pool->workMutex);
    if (!pool->head) {
        pool->head = work;
        pool->tail = pool->head;
    } else {
        work->next = pool->head;
        pool->head = work;
    }
    pthread_cond_broadcast(&pool->workCond);
    pthread_mutex_unlock(&pool->workMutex);
    return 1;
}

void thread_pool_join(thread_pool_t *pool) {
    pthread_mutex_lock(&pool->workMutex);
    for (;;)
        if (pool->head ||
            (!pool->kill && pool->workingCount > 0) ||
            (pool->kill && pool->threadCount > 0))
            pthread_cond_wait(&pool->workingCond, &pool->workMutex);
        else
            break;
    pthread_mutex_unlock(&pool->workMutex);
}

thread_queue_t* thread_queue(void) {
    thread_queue_t *queue = malloc(sizeof(thread_queue_t));
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    pthread_mutex_init(&queue->readLock, NULL);
    pthread_mutex_init(&queue->writeLock, NULL);
    pthread_mutex_lock(&queue->readLock);
    return queue;
}

void thread_queue_push(thread_queue_t *queue, void *data) {
    thread_queue_entry_t *item = malloc(sizeof(thread_queue_entry_t));
    item->data = data;
    item->next = NULL;

    pthread_mutex_lock(&queue->writeLock);
    if (!queue->head) {
        queue->head = item;
        queue->tail = queue->head;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->count++;

    pthread_mutex_unlock(&queue->readLock);
    pthread_mutex_unlock(&queue->writeLock);
}

void* thread_queue_pop(thread_queue_t *queue) {
    if (!queue->head)
        return NULL;

    pthread_mutex_lock(&queue->readLock);
    pthread_mutex_lock(&queue->writeLock);

    void *result = queue->head->data;
    thread_queue_entry_t *tmp = queue->head;
    if (!(queue->head = tmp->next))
        queue->tail = NULL;
    free(tmp);

    if (--queue->count)
        pthread_mutex_unlock(&queue->readLock);
    pthread_mutex_unlock(&queue->writeLock);

    return result;
}

void thread_queue_destroy(thread_queue_t *queue) {
    // TODO: Handle mutexes + clear queue first
    pthread_mutex_destroy(&queue->readLock);
    pthread_mutex_destroy(&queue->writeLock);
    free(queue);
}
