# c-pthread-pool

C thread pool implementation using POSIX threads (pthreads).

## Usage

```c
#include "threadpool.h"

struct worker_arg {
    char* filepath;
    int writefd;
};

// Create the thread pool with 8 threads
threadpool_t* tpool = create_thread_pool(8);

// Enqueue work for the thread pool
struct worker_arg warg;
warg.filepath = buffer;
warg.writefd = writefd;
enqueue_thread_pool(tpool, file_worker, &warg);

void* file_worker(void* arg) {
    // Parse the worker_arg
    struct worker_arg* warg = (struct worker_arg*) arg;
}
```