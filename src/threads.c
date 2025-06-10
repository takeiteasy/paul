/* jeff/events.c -- https://github.com/takeiteasy/jeff

 Copyright (C) 2025  George Watson

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

#include "jeff.h"

#ifndef JEFF_NO_THREADS
static int thread_pool_worker(void *arg) {
    thread_pool_t *pool = (thread_pool_t*)arg;
    while (!pool->kill) {
        while (!pool->head && !pool->kill)
            paul_cnd_wait(&pool->workCond, &pool->workMutex);
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

        paul_mtx_lock(&pool->workMutex);
        work->func(work->arg);
        free(work);
        paul_mtx_lock(&pool->workMutex);
        if (!pool->kill && !--pool->workingCount && pool->head)
            paul_cnd_signal(&pool->workingCond);
        paul_mtx_unlock(&pool->workMutex);
    }

    pool->threadCount--;
    paul_cnd_signal(&pool->workingCond);
    paul_mtx_unlock(&pool->workMutex);
    return 0;
}

thread_pool_t* thread_pool(size_t maxThreads) {
    if (!maxThreads)
        return NULL;
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));
    paul_mtx_init(&pool->workMutex, PAUL_MTX_PLAIN);
    paul_cnd_init(&pool->workCond);
    paul_cnd_init(&pool->workingCond);
    pool->head = pool->tail = NULL;
    pool->threadCount = maxThreads;

    paul_thrd_t _thrd;
    for (int i = 0; i < maxThreads; i++) {
        paul_thrd_create(&_thrd, thread_pool_worker, (void*)pool);
        paul_thrd_detach(_thrd);
    }
    return pool;
}

void thread_pool_destroy(thread_pool_t *pool) {
    paul_mtx_lock(&pool->workMutex);
    thread_work_t *work = pool->head;
    while (work) {
        thread_work_t *tmp = work->next;
        free(work);
        work = tmp;
    }
    paul_mtx_unlock(&pool->workMutex);
    thread_pool_join(pool);
    paul_mtx_destroy(&pool->workMutex);
    paul_cnd_destroy(&pool->workCond);
    paul_cnd_destroy(&pool->workingCond);
    free(pool);
}

int thread_pool_push_work(thread_pool_t *pool, void(*func)(void*), void *arg) {
    thread_work_t *work = malloc(sizeof(thread_work_t));
    work->arg = arg;
    work->func = func;
    work->next = NULL;
    paul_mtx_lock(&pool->workMutex);
    if (!pool->head) {
        pool->head = work;
        pool->tail = pool->head;
    } else {
        pool->tail->next = work;
        pool->tail = work;
    }
    paul_cnd_broadcast(&pool->workCond);
    paul_mtx_unlock(&pool->workMutex);
    return 1;
}

int thread_pool_push_work_priority(thread_pool_t *pool, void(*func)(void*), void *arg) {
     thread_work_t *work = malloc(sizeof(thread_work_t));
    work->arg = arg;
    work->func = func;
    work->next = NULL;
    paul_mtx_lock(&pool->workMutex);
    if (!pool->head) {
        pool->head = work;
        pool->tail = pool->head;
    } else {
        work->next = pool->head;
        pool->head = work;
    }
    paul_cnd_broadcast(&pool->workCond);
    paul_mtx_unlock(&pool->workMutex);
    return 1;
}

void thread_pool_join(thread_pool_t *pool) {
    paul_mtx_lock(&pool->workMutex);
    for (;;)
        if (pool->head ||
            (!pool->kill && pool->workingCount > 0) ||
            (pool->kill && pool->threadCount > 0))
            paul_cnd_wait(&pool->workingCond, &pool->workMutex);
        else
            break;
    paul_mtx_unlock(&pool->workMutex);
}

thread_queue_t* thread_queue(void) {
    thread_queue_t *queue = malloc(sizeof(thread_queue_t));
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    paul_mtx_init(&queue->readLock, PAUL_MTX_PLAIN);
    paul_mtx_init(&queue->writeLock, PAUL_MTX_PLAIN);
    paul_mtx_lock(&queue->readLock);
    return queue;
}

void thread_queue_push(thread_queue_t *queue, void *data) {
    thread_queue_entry_t *item = malloc(sizeof(thread_queue_entry_t));
    item->data = data;
    item->next = NULL;

    paul_mtx_lock(&queue->writeLock);
    if (!queue->head) {
        queue->head = item;
        queue->tail = queue->head;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->count++;

    paul_mtx_unlock(&queue->readLock);
    paul_mtx_unlock(&queue->writeLock);
}

void* thread_queue_pop(thread_queue_t *queue) {
    if (!queue->head)
        return NULL;

    paul_mtx_lock(&queue->readLock);
    paul_mtx_lock(&queue->writeLock);

    void *result = queue->head->data;
    thread_queue_entry_t *tmp = queue->head;
    if (!(queue->head = tmp->next))
        queue->tail = NULL;
    free(tmp);

    if (--queue->count)
        paul_mtx_unlock(&queue->readLock);
    paul_mtx_unlock(&queue->writeLock);
    return result;
}

void thread_queue_destroy(thread_queue_t *queue) {
    // TODO: Handle mutexes + clear queue first
    paul_mtx_destroy(&queue->readLock);
    paul_mtx_destroy(&queue->writeLock);
    free(queue);
}
#endif
