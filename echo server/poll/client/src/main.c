#include <sys/types.h>	// for socket
#include <sys/socket.h>	// for socket
#include <netinet/in.h> // for struct sockaddr_in
#include <arpa/inet.h>	// for htons inet_addr inet_ntoa

#include <unistd.h>		// for read write
#include <stdio.h>		// for perror
#include <stdlib.h>		// for exit
#include <strings.h>	// for bzero
#include <errno.h>		// for errno

#define IP_ADDRESS "192.168.4.20"
#define PORT 9734
#define MAX_LEN 1024

#define ERR_EXIT( err_msg )\
	do{\
		perror(err_msg);\
		exit(EXIT_FAILURE);\
	}while(0)

#define ERR_EXIT1( err_msg )\
	do{\
		fprintf( stderr , "%s\n" , err_msg );\
		exit(EXIT_FAILURE);\
	}while(0)

static void do_service( int sfd );

int main( void )
{
	int sfd = 0;
	struct sockaddr_in server_addr;
	socklen_t server_len = 0;

	int ret = 0;

	// create a socket
	sfd = socket( AF_INET , SOCK_STREAM , 0 );
	if( -1 == sfd )
		ERR_EXIT( "socket error" );

	// set the server address
	bzero( &server_addr , 0 );
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( PORT );
	server_addr.sin_addr.s_addr = inet_addr( IP_ADDRESS );

	// connect to server
	server_len = ( socklen_t )sizeof( server_addr );
	ret = connect( sfd , (struct sockaddr*)&server_addr , server_len );
	if( -1 == ret )
		ERR_EXIT( "connect error" );

	// do service
	do_service( sfd );

	exit( EXIT_SUCCESS );

}// main

static void do_service( int sfd )
{
	char buf[MAX_LEN];
	int nread = 0;
	int nwrite = 0;

	if( sfd < 0 )
		ERR_EXIT1( "Argument to do_service is not valid!" );
	
	while( printf( "Please input a string : " ) , fflush( stdout ) , nread = read( 0 , buf , MAX_LEN ) )
	{
		if( -1 == nread )
		{
			if( EINTR == errno )
				continue;
			else
				ERR_EXIT( "read error" );
		}

		// write to server
		nwrite = write( sfd , buf , nread );
		if( -1 == nwrite )
		{
			if( EINTR == errno )	
				continue;
			else
				ERR_EXIT( "write error" );
		}
		if( 0 == nwrite )			
			continue;

		// read from server
		nread = read( sfd , buf , nwrite );
		if( -1 == nread )
		{
			if( EINTR == errno )	
				continue;
			else
				ERR_EXIT( "read error" );
		}
		if( 0 == nread )			
			break;

		printf( "Recv from server : " );
		fflush( stdout );
		nwrite = write( 1 , buf , nread );
		if( -1 == nwrite )			
		{
			if( EINTR == errno )
				continue;
			else
				ERR_EXIT( "write error" );
		}
		if( 0 == nwrite )			
			continue;
	}

	close( sfd );

}// do_service
