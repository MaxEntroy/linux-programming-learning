#define THREADPOOL_H_
#include <pthread.h>
#include "common.h"
#include "threadpool_queue.h"

typedef void* (*handle_t)(void*);

typedef struct threadpool {
	pthread_t* pool_arr;
	int pool_num;
	handle_t pool_handle;
	
	que_t pool_que;

}threadpool_t;

void threadpool_init( threadpool_t* ppool, int thread_num, handle_t thread_handle, int que_size  );
void threadpool_destroy( threadpool_t* ppool );

void threadpool_start( threadpool_t* ppool );
void threadpool_stop( threadpool_t* ppool );

void threadpool_put( threadpool_t* ppool, task_t* val );
void threadpool_get( threadpool_t* ppool, task_t* val );

#endif
