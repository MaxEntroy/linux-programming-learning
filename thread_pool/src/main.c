#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include "threadpool.h"
#define LEN 128

static void* worker(void*);
static int process_task( task_t* val );

int main( int argc, char* argv[] ){
	
	fd_set readfds;
	fd_set tmpfds;
	struct timeval tv;
	int ret = 0;
	int nread = 0;
	char buf[LEN];

	if( argc != 3 ){
		fprintf( stderr, "Usage: argv[1] is thread num, argv[2] is que size!\n" );
		exit(EXIT_FAILURE);
	}

	threadpool_t pool;
	threadpool_init( &pool, atoi(argv[1]), worker, atoi(argv[2]) );
	threadpool_start( &pool );
	
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);

	while(1) {
		tmpfds = readfds;
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		
		ret = select( STDIN_FILENO+1, &tmpfds, NULL, NULL, &tv );
		
		if( -1 == ret ) {
			perror( "select" );
			exit( EXIT_FAILURE );
		}
		else if( !ret ) {
			printf( "No input data in 3 seconds!\n" );
		}
		else {
			if( FD_ISSET(STDIN_FILENO, &tmpfds) ) {
				memset(buf, 0, sizeof(buf));
				nread = read( STDIN_FILENO, buf, LEN-1 );
				if( -1 == nread ) {
					perror( "read" );
					exit( EXIT_FAILURE );
				}
				if( !nread ) break;
				else{
					buf[nread-1] = 0;
					task_t val;
					sscanf(buf, "%d %d", &val.left, &val.right);
					
					threadpool_put( &pool, &val );
				}
			}
		}
	}
	threadpool_stop( &pool );
	threadpool_destroy( &pool );
	exit(EXIT_SUCCESS);

}// main

static void* worker(void* arg) {
	assert(arg);
	
	threadpool_t* ppool = (threadpool_t*)(arg);
	task_t val;
	
	while( 1 ){
		threadpool_get( ppool, &val );
		if( !que_valid( &(ppool->pool_que) ) ) break;
		int res = process_task(&val);
		printf( "%lu execute the task: %d\n", pthread_self(), res );
		sleep(1);
	}
	pthread_exit(NULL);
}// thread_handle

static int process_task( task_t* val ) {
	return val->left + val->right;
}// process_task
