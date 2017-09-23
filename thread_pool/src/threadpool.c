/*************************************************************************
	> File Name: threadpool.c
	> Author: Kang
	> Mail:likang@tju.edu.cn 
	> Created Time: 2017年09月21日 星期四 14时05分02秒
 ************************************************************************/
#include "threadpool.h"
#include "threadpool_queue.h"
#include <assert.h>
#include <stdlib.h>

void threadpool_init( threadpool_t* ppool, 
		              int thread_num, 
					  handle_t thread_handle, 
					  int que_size) {
	assert( ppool && thread_num>0 && thread_handle && que_size > 0 );
	
	ppool->pool_arr = (pthread_t*)malloc( sizeof(pthread_t) * thread_num );
	assert( ppool->pool_arr );

	ppool->pool_num = thread_num;
	ppool->pool_handle = thread_handle;
	
	que_init( &(ppool->pool_que), que_size );
}// threadpool_init

void threadpool_destroy( threadpool_t* ppool ) {
	assert( ppool );
	
	free(ppool->pool_arr);

	que_destroy( &(ppool->pool_que) );

}// threadpool_destroy

void threadpool_start( threadpool_t* ppool ) {
	int i = 0;
	assert( ppool );
	
	que_start( &(ppool->pool_que) );
	for( i = 0; i < ppool->pool_num; ++i ){
		if( pthread_create( ppool->pool_arr + i, NULL, ppool->pool_handle, (void*)ppool ) != 0 ) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}

}// threadpool_start

void threadpool_stop( threadpool_t* ppool ) {
	int i = 0;
	assert( ppool );
	
	que_stop( &(ppool->pool_que) );
	que_wake_all( &(ppool->pool_que) );
	for( i = 0; i < ppool->pool_num; ++i ) {
		if( pthread_join( ppool->pool_arr[i], NULL ) != 0 ) {
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}
	}
}// threadpool_stop

void threadpool_put( threadpool_t* ppool, task_t* val ) {
	assert( ppool );
	que_push( &(ppool->pool_que), val );
}// threadpool_put

void threadpool_get( threadpool_t* ppool, task_t* val ) {
	assert( ppool );
	que_pop( &(ppool->pool_que), val );
}// threadpool_get
