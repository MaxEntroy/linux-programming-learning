/*************************************************************************
	> File Name: threadpool_queue.h
	> Author: Kang
	> Mail:likang@tju.edu.cn 
 ************************************************************************/
#ifndef THREADPOOL_QUEUE_H_
#define THREADPOOL_QUEUE_H_
#include <pthread.h>
#include "common.h"

typedef struct que {
	task_t* que_arr;
	int que_front;
	int que_rear;
	int que_size;
	pthread_mutex_t que_mtx;
	pthread_cond_t que_cond_pro; // que full block
	pthread_cond_t que_cond_con; // que empty block
	int que_flag;
}que_t;

void que_init( que_t* pque, int size );
void que_destroy( que_t* pque );

void que_push( que_t* pque, task_t* val );
void que_pop( que_t* pque, task_t* val );

Status que_empty( que_t* pque ); // return true iff que is empty
Status que_full( que_t* pque );  // return true iff que is full

void que_start( que_t* pque );
void que_stop( que_t* pque );

void que_wake_all( que_t* pque );
Status que_valid( que_t* pque );

#endif
