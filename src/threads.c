/* paul/events.c -- https://github.com/takeiteasy/paul

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

#include "paul.h"

#ifndef PAUL_NO_THREADS
static int paul_thrd_pool_worker(void *arg) {
    paul_thrd_pool_t *pool = (paul_thrd_pool_t*)arg;
    while (!pool->kill) {
        while (!pool->head && !pool->kill)
            hal_cnd_wait(&pool->workCond, &pool->workMutex);
        if (pool->kill)
            break;

        paul_thrd_work_t *work = pool->head;
        if (!work)
            break;
        if (!work->next) {
            pool->head = NULL;
            pool->tail = NULL;
        } else
            pool->head = work->next;
        pool->workingCount++;

        hal_mtx_lock(&pool->workMutex);
        work->func(work->arg);
        free(work);
        hal_mtx_lock(&pool->workMutex);
        if (!pool->kill && !--pool->workingCount && pool->head)
            hal_cnd_signal(&pool->workingCond);
        hal_mtx_unlock(&pool->workMutex);
    }

    pool->threadCount--;
    hal_cnd_signal(&pool->workingCond);
    hal_mtx_unlock(&pool->workMutex);
    return 0;
}

bool paul_thrd_pool(size_t maxThreads, paul_thrd_pool_t *pool) {
    if (!maxThreads || !pool)
        return false;
    hal_mtx_init(&pool->workMutex, HAL_MTX_PLAIN);
    hal_cnd_init(&pool->workCond);
    hal_cnd_init(&pool->workingCond);
    pool->head = pool->tail = NULL;
    pool->threadCount = maxThreads;
    hal_thrd_t _thrd;
    for (int i = 0; i < maxThreads; i++) {
        hal_thrd_create(&_thrd, paul_thrd_pool_worker, (void*)pool);
        hal_thrd_detach(_thrd);
    }
    return true;
}

void paul_thrd_pool_destroy(paul_thrd_pool_t *pool) {
    if (!pool)
        return;
    hal_mtx_lock(&pool->workMutex);
    paul_thrd_work_t *work = pool->head;
    while (work) {
        paul_thrd_work_t *tmp = work->next;
        free(work);
        work = tmp;
    }
    hal_mtx_unlock(&pool->workMutex);
    paul_thrd_pool_join(pool);
    hal_mtx_destroy(&pool->workMutex);
    hal_cnd_destroy(&pool->workCond);
    hal_cnd_destroy(&pool->workingCond);
    memset(pool, 0, sizeof(paul_thrd_pool_t));
}

int paul_thrd_pool_push_work(paul_thrd_pool_t *pool, void(*func)(void*), void *arg) {
    paul_thrd_work_t *work = malloc(sizeof(paul_thrd_work_t));
    work->arg = arg;
    work->func = func;
    work->next = NULL;
    hal_mtx_lock(&pool->workMutex);
    if (!pool->head) {
        pool->head = work;
        pool->tail = pool->head;
    } else {
        pool->tail->next = work;
        pool->tail = work;
    }
    hal_cnd_broadcast(&pool->workCond);
    hal_mtx_unlock(&pool->workMutex);
    return 1;
}

int paul_thrd_pool_push_work_priority(paul_thrd_pool_t *pool, void(*func)(void*), void *arg) {
     paul_thrd_work_t *work = malloc(sizeof(paul_thrd_work_t));
    work->arg = arg;
    work->func = func;
    work->next = NULL;
    hal_mtx_lock(&pool->workMutex);
    if (!pool->head) {
        pool->head = work;
        pool->tail = pool->head;
    } else {
        work->next = pool->head;
        pool->head = work;
    }
    hal_cnd_broadcast(&pool->workCond);
    hal_mtx_unlock(&pool->workMutex);
    return 1;
}

void paul_thrd_pool_join(paul_thrd_pool_t *pool) {
    hal_mtx_lock(&pool->workMutex);
    for (;;)
        if (pool->head ||
            (!pool->kill && pool->workingCount > 0) ||
            (pool->kill && pool->threadCount > 0))
            hal_cnd_wait(&pool->workingCond, &pool->workMutex);
        else
            break;
    hal_mtx_unlock(&pool->workMutex);
}

bool paul_thrd_queue(paul_thrd_queue_t *queue) {
    if (!queue)
        return false;
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    hal_mtx_init(&queue->readLock, HAL_MTX_PLAIN);
    hal_mtx_init(&queue->writeLock, HAL_MTX_PLAIN);
    hal_mtx_lock(&queue->readLock);
    return true;
}

void paul_thrd_queue_push(paul_thrd_queue_t *queue, void *data) {
    paul_thrd_queue_entry_t *item = malloc(sizeof(paul_thrd_queue_entry_t));
    item->data = data;
    item->next = NULL;

    hal_mtx_lock(&queue->writeLock);
    if (!queue->head) {
        queue->head = item;
        queue->tail = queue->head;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->count++;

    hal_mtx_unlock(&queue->readLock);
    hal_mtx_unlock(&queue->writeLock);
}

void* paul_thrd_queue_pop(paul_thrd_queue_t *queue) {
    if (!queue->head)
        return NULL;

    hal_mtx_lock(&queue->readLock);
    hal_mtx_lock(&queue->writeLock);

    void *result = queue->head->data;
    paul_thrd_queue_entry_t *tmp = queue->head;
    if (!(queue->head = tmp->next))
        queue->tail = NULL;
    free(tmp);

    if (--queue->count)
        hal_mtx_unlock(&queue->readLock);
    hal_mtx_unlock(&queue->writeLock);
    return result;
}

void paul_thrd_queue_destroy(paul_thrd_queue_t *queue) {
    if (!queue)
        return;
    // TODO: Handle mutexes + clear queue first
    hal_mtx_destroy(&queue->readLock);
    hal_mtx_destroy(&queue->writeLock);
    memset(queue, 0, sizeof(paul_thrd_queue_t));
}
#endif
