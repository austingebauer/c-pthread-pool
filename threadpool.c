#include "threadpool.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

static threadpool_work_t* threadpool_work_create(thread_func_t func, void* arg) {
    threadpool_work_t* work;
    if (func == NULL) {
        return NULL;
    }

    work = malloc(sizeof(*work));
    work->func = func;
    work->arg = arg;
    work->next = NULL;
    return work;
}

static void threadpool_work_destroy(threadpool_work_t* work) {
    if (work == NULL) {
        return;
    }
    free(work);
}

static threadpool_work_t* threadpool_work_get(threadpool_t* tm) {
    if (tm == NULL) {
        return NULL;
    }

    threadpool_work_t* work;
    work = tm->first;
    if (work == NULL) {
        return NULL;
    }

    if (work->next == NULL) {
        tm->first = NULL;
        tm->last = NULL;
    } else {
        tm->first = work->next;
    }

    return work;
}

static void* threadpool_worker(void* arg) {
    threadpool_t* tm = arg;
    threadpool_work_t* work;

    while (1) {
        pthread_mutex_lock(&(tm->mutex));

        while (tm->first == NULL && !tm->stop)
            pthread_cond_wait(&(tm->cond), &(tm->mutex));

        if (tm->stop) {
            break;
        }

        work = threadpool_work_get(tm);
        tm->working_cnt++;
        pthread_mutex_unlock(&(tm->mutex));

        if (work != NULL) {
            work->func(work->arg);
            threadpool_work_destroy(work);
        }

        pthread_mutex_lock(&(tm->mutex));
        tm->working_cnt--;
        if (!tm->stop && tm->working_cnt == 0 && tm->first == NULL) {
            pthread_cond_signal(&(tm->working_cond));
        }
        pthread_mutex_unlock(&(tm->mutex));
    }

    tm->thread_cnt--;
    pthread_cond_signal(&(tm->working_cond));
    pthread_mutex_unlock(&(tm->mutex));
    return NULL;
}

threadpool_t* create_thread_pool(size_t num) {
    threadpool_t* tm;
    pthread_t thread;
    size_t i;

    if (num == 0) {
        num = 2;
    }

    tm = calloc(1, sizeof(*tm));
    tm->thread_cnt = num;

    pthread_mutex_init(&(tm->mutex), NULL);
    pthread_cond_init(&(tm->cond), NULL);
    pthread_cond_init(&(tm->working_cond), NULL);

    tm->first = NULL;
    tm->last = NULL;

    for (i = 0; i < num; i++) {
        pthread_create(&thread, NULL, threadpool_worker, tm);
        pthread_detach(thread);
    }

    return tm;
}

void destroy_thread_pool(threadpool_t* tm) {
    threadpool_work_t* work;
    threadpool_work_t* work2;

    if (tm == NULL) {
        return;
    }

    pthread_mutex_lock(&(tm->mutex));
    work = tm->first;
    while (work != NULL) {
        work2 = work->next;
        threadpool_work_destroy(work);
        work = work2;
    }
    tm->stop = true;
    pthread_cond_broadcast(&(tm->cond));
    pthread_mutex_unlock(&(tm->mutex));

    pthread_mutex_lock(&(tm->mutex));
    while (1) {
        if ((!tm->stop && tm->working_cnt != 0) || (tm->stop && tm->thread_cnt != 0)) {
            pthread_cond_wait(&(tm->working_cond), &(tm->mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tm->mutex));

    pthread_mutex_destroy(&(tm->mutex));
    pthread_cond_destroy(&(tm->cond));
    pthread_cond_destroy(&(tm->working_cond));

    free(tm);
}

bool enqueue_thread_pool(threadpool_t* tm, thread_func_t func, void* arg) {
    threadpool_work_t* work;

    if (tm == NULL) {
        return false;
    }

    work = threadpool_work_create(func, arg);
    if (work == NULL) {
        return false;
    }

    pthread_mutex_lock(&(tm->mutex));
    if (tm->first == NULL) {
        tm->first = work;
        tm->last = tm->first;
    } else {
        tm->last->next = work;
        tm->last = work;
    }

    pthread_cond_broadcast(&(tm->cond));
    pthread_mutex_unlock(&(tm->mutex));
    return true;
}
