#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define CONSUMER_NUM 3

void* producer( void* );
void* consumer( void* );

void err_msg( const char* msg ){ perror(msg); exit(EXIT_FAILURE); }

int ticket_num; 
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pro = PTHREAD_COND_INITIALIZER;
pthread_cond_t con = PTHREAD_COND_INITIALIZER;

int main( void ){

	int ret = 0;
	int i = 0;
	pthread_t pro_id;

	pthread_t con_id_arr[CONSUMER_NUM];

	ticket_num = 10;

	// create producer
	ret = pthread_create( &pro_id, NULL, producer, NULL );
	if( ret != 0 )
		err_msg( "pthread_create()" );

	// create consumer
	for( i = 0; i < CONSUMER_NUM; ++i ){
		ret = pthread_create( con_id_arr + i, NULL, consumer, NULL );
		if( ret != 0 )
			err_msg( "pthread_create()" );
	}

	// join producer
	ret = pthread_join( pro_id, NULL );	
	if( ret != 0 )
		err_msg( "pthread_join()" );

	// join consumer
	for( i = 0; i < CONSUMER_NUM; ++i ){
		ret = pthread_join( *(con_id_arr + i), NULL);
		if( ret != 0 )
			err_msg( "pthread_join()" );
	}
	
	printf( "Thread has joined!\n" );
	exit(EXIT_SUCCESS);
}

void* producer( void* arg){
	while(1){
		pthread_mutex_lock( &mtx );
		
		while( ticket_num > 0 ){
			// no-op
			pthread_cond_wait(&pro, &mtx);
		}
		
		ticket_num += 10;
		pthread_cond_signal(&con);
		printf( "%lu supply 10 tickets!\n", pthread_self() );
		
		pthread_mutex_unlock( &mtx );
	}
}
void* consumer( void* arg){
	while(1){
		pthread_mutex_lock( &mtx );
		
		while( 0 == ticket_num ){
			// no-op
			pthread_cond_signal(&pro);
			pthread_cond_wait( &con, &mtx );
		}
		
		ticket_num--;
		printf( "%lu buys a ticket, ticket_num = %d.\n", pthread_self(), ticket_num+1 );
		
		pthread_mutex_unlock( &mtx );
		sleep(1); // for fair
	}
}
