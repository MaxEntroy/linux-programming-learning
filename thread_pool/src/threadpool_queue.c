/*************************************************************************
	> File Name: threadpool_queue.c
	> Author: Kang
	> Mail:likang@tju.edu.cn 
	> Created Time: 2017年09月21日 星期四 12时58分34秒
 ************************************************************************/
#include "threadpool_queue.h"
#include <assert.h>
#include <stdlib.h> 
#include <errno.h>

void que_init( que_t* pque, int size ) {
	assert(pque && size > 0);

	pque->que_arr = (task_t*)malloc( size * sizeof(que_t) );
	assert(pque->que_arr);

	pque->que_size = size;
	
	pque->que_front = pque->que_rear = 0;

	if( ( pthread_mutex_init( &(pque->que_mtx) , NULL ) ) != 0 ){
		perror("pthread_mutex_init");
		exit(EXIT_FAILURE);
	}

	if( ( pthread_cond_init( &(pque->que_cond_pro), NULL ) ) != 0 ){
		perror("pthread_cond_init");
		exit(EXIT_FAILURE);
	}
	if( ( pthread_cond_init( &(pque->que_cond_con), NULL ) ) != 0 ){
		perror("pthread_cond_init");
		exit(EXIT_FAILURE);
	}

	pque->que_flag = 0;
}// que_init

void que_destroy( que_t* pque ) {
	assert(pque);
	
	assert(pque->que_arr);
	free(pque->que_arr);
	
	if( ( pthread_mutex_destroy( &(pque->que_mtx) ) ) != 0 ) {
		perror("pthread_mutex_destroy");
		exit(EXIT_FAILURE);
	}
	if( ( pthread_cond_destroy( &(pque->que_cond_pro) ) ) != 0 ) {
		perror("pthread_mutex_destroy");
		exit(EXIT_FAILURE);
	}
	if( ( pthread_cond_destroy( &(pque->que_cond_con) ) ) != 0 ) {
		perror("pthread_mutex_destroy");
		exit(EXIT_FAILURE);
	}
}// que_destroy

void que_push( que_t* pque, task_t* val ) {
	assert(pque && val);

	pthread_mutex_lock( &(pque->que_mtx) );
	while( que_full(pque) && que_valid(pque) ) {
		pthread_cond_wait( &(pque->que_cond_pro), &(pque->que_mtx) );
	}
	
	if( que_valid(pque) ) {

		pque->que_arr[pque->que_rear] = *val;
		pque->que_rear = (pque->que_rear + 1) % pque->que_size;

		pthread_cond_signal(&(pque->que_cond_con));
	}

	pthread_mutex_unlock( &(pque->que_mtx) );
}// que_push

void que_pop( que_t* pque, task_t* val ) {
	assert(pque && val );

	pthread_mutex_lock( &(pque->que_mtx) );
	
	while( que_empty(pque) && que_valid(pque) ) {
		pthread_cond_wait( &(pque->que_cond_con), &(pque->que_mtx) );
	}

	if( que_valid(pque) ) {
		*val = pque->que_arr[pque->que_front];
		pque->que_front = (pque->que_front + 1) % pque->que_size;
		pthread_cond_signal(&(pque->que_cond_pro));
	}

	pthread_mutex_unlock( &(pque->que_mtx) );
}// que_pop

Status que_empty( que_t* pque ) {
	assert(pque);
	return pque->que_front == pque->que_rear;
}// que_empty

Status que_full( que_t* pque ) {
	assert(pque);
	return (pque->que_rear + 1) % pque->que_size == pque->que_front;
}// que_full

void que_start( que_t* pque ) {
	assert(pque);
	pque->que_flag = 1;
}// que_start

void que_stop( que_t* pque ) {
	assert(pque);
	pque->que_flag = 0;
}// que_stop

void que_wake_all( que_t* pque ) {
	assert( pque );
	pthread_cond_broadcast( &(pque->que_cond_con) );
	pthread_cond_broadcast( &(pque->que_cond_pro) );
}// que_wake_all

Status que_valid( que_t* pque ) {
	assert(pque);
	return pque->que_flag;
}// que_valid
