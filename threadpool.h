#ifndef C_PTHREAD_POOL_THREADPOOL_H
#define C_PTHREAD_POOL_THREADPOOL_H

#include <stdbool.h>
#include <stddef.h>

struct threadpool_work {
    struct threadpool_work* next;
    thread_func_t func;
    void* arg;
};
typedef struct threadpool_work threadpool_work_t;

struct threadpool {
    bool stop;
    size_t working_cnt;
    size_t thread_cnt;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_cond_t working_cond;
    threadpool_work_t* last;
    threadpool_work_t* first;
};
typedef struct threadpool threadpool_t;

typedef void* (*thread_func_t)(void* arg);
threadpool_t* create_thread_pool(size_t num);
void destroy_thread_pool(threadpool_t* tm);
bool enqueue_thread_pool(threadpool_t* tm, thread_func_t func, void* arg);

#endif// C_PTHREAD_POOL_THREADPOOL_H
