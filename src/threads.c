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
static int jeff_thrd_pool_worker(void *arg) {
    jeff_thrd_pool_t *pool = (jeff_thrd_pool_t*)arg;
    while (!pool->kill) {
        while (!pool->head && !pool->kill)
            paul_cnd_wait(&pool->workCond, &pool->workMutex);
        if (pool->kill)
            break;

        jeff_thrd_work_t *work = pool->head;
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

bool jeff_thrd_pool(size_t maxThreads, jeff_thrd_pool_t *pool) {
    if (!maxThreads || !pool)
        return false;
    paul_mtx_init(&pool->workMutex, PAUL_MTX_PLAIN);
    paul_cnd_init(&pool->workCond);
    paul_cnd_init(&pool->workingCond);
    pool->head = pool->tail = NULL;
    pool->threadCount = maxThreads;
    paul_thrd_t _thrd;
    for (int i = 0; i < maxThreads; i++) {
        paul_thrd_create(&_thrd, jeff_thrd_pool_worker, (void*)pool);
        paul_thrd_detach(_thrd);
    }
    return true;
}

void jeff_thrd_pool_destroy(jeff_thrd_pool_t *pool) {
    if (!pool)
        return;
    paul_mtx_lock(&pool->workMutex);
    jeff_thrd_work_t *work = pool->head;
    while (work) {
        jeff_thrd_work_t *tmp = work->next;
        free(work);
        work = tmp;
    }
    paul_mtx_unlock(&pool->workMutex);
    jeff_thrd_pool_join(pool);
    paul_mtx_destroy(&pool->workMutex);
    paul_cnd_destroy(&pool->workCond);
    paul_cnd_destroy(&pool->workingCond);
    memset(pool, 0, sizeof(jeff_thrd_pool_t));
}

int jeff_thrd_pool_push_work(jeff_thrd_pool_t *pool, void(*func)(void*), void *arg) {
    jeff_thrd_work_t *work = malloc(sizeof(jeff_thrd_work_t));
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

int jeff_thrd_pool_push_work_priority(jeff_thrd_pool_t *pool, void(*func)(void*), void *arg) {
     jeff_thrd_work_t *work = malloc(sizeof(jeff_thrd_work_t));
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

void jeff_thrd_pool_join(jeff_thrd_pool_t *pool) {
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

bool jeff_thrd_queue(jeff_thrd_queue_t *queue) {
    if (!queue)
        return false;
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    paul_mtx_init(&queue->readLock, PAUL_MTX_PLAIN);
    paul_mtx_init(&queue->writeLock, PAUL_MTX_PLAIN);
    paul_mtx_lock(&queue->readLock);
    return true;
}

void jeff_thrd_queue_push(jeff_thrd_queue_t *queue, void *data) {
    jeff_thrd_queue_entry_t *item = malloc(sizeof(jeff_thrd_queue_entry_t));
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

void* jeff_thrd_queue_pop(jeff_thrd_queue_t *queue) {
    if (!queue->head)
        return NULL;

    paul_mtx_lock(&queue->readLock);
    paul_mtx_lock(&queue->writeLock);

    void *result = queue->head->data;
    jeff_thrd_queue_entry_t *tmp = queue->head;
    if (!(queue->head = tmp->next))
        queue->tail = NULL;
    free(tmp);

    if (--queue->count)
        paul_mtx_unlock(&queue->readLock);
    paul_mtx_unlock(&queue->writeLock);
    return result;
}

void jeff_thrd_queue_destroy(jeff_thrd_queue_t *queue) {
    if (!queue)
        return;
    // TODO: Handle mutexes + clear queue first
    paul_mtx_destroy(&queue->readLock);
    paul_mtx_destroy(&queue->writeLock);
    memset(queue, 0, sizeof(jeff_thrd_queue_t));
}
#endif
