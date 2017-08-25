#include <sys/types.h>	// for socket
#include <sys/socket.h>	// for socket
#include <netinet/in.h> // for struct sockaddr_in htons
#include <arpa/inet.h>	// for inet_addr

#include <stdio.h>		// for perror
#include <stdlib.h>		// for exit
#include <strings.h>	// for bzero
#include <errno.h>		// for errno
#include <assert.h>		// for assert

#define ERR_EXIT( msg )\
	do{\
		perror( msg );\
		exit( EXIT_FAILURE );\
	}while(0)

#define ERR_EXIT1( msg )\
	do{\
		fprintf( stderr , "%s\n" ,  msg );\
		exit(EXIT_FAILURE);\
	}while(0)

#define IP_ADDRESS "192.168.4.20"
#define PORT 9734
#define MAXLEN 128

static void do_service( int sfd );

int main( void )
{
	int sfd = 0;
	struct sockaddr_in server_addr;
	socklen_t server_len = 0;

	int ret = 0;

	// create the socket
	sfd = socket( AF_INET , SOCK_STREAM , 0 );
	if( -1 == sfd )
		ERR_EXIT( "socket error" );

	// set the server address
	bzero( &server_addr , 0 );
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( PORT );
	server_addr.sin_addr.s_addr = inet_addr( IP_ADDRESS );
	
	// connect to server
	server_len = (socklen_t)sizeof( server_addr );
	ret = connect( sfd , (struct sockaddr*)&server_addr , server_len );
	if( -1 == ret )
		ERR_EXIT( "connect error" );

	// do service
	do_service( sfd );

	exit( EXIT_SUCCESS );
}// main

static void do_service( int sfd )
{	
	char buf[MAXLEN];
	int nread = 0;
	int nwrite = 0;

	if( sfd < 0 )
		ERR_EXIT1( "Argument to do_servive is not valid!" );

	while( printf( "Please input a string : " ) , fflush( stdout ) , nread = read( 0 , buf , MAXLEN ) )
	{
		if( -1 == nread )
		{
			if( EINTR == errno )
				continue;
			else
				ERR_EXIT( "read error" );
		}
		
		// write to server
write_label1:
		nwrite = write( sfd , buf , nread );
		if( 0 == nwrite )
			break;
		if( -1 == nwrite )
		{
			if( EINTR == errno )
				goto write_label1;
			else
				ERR_EXIT( "write error" );
		}

		// read from server
read_lable:
		nread = read( sfd , buf , MAXLEN );
		if( 0 == nread )
			break;
		if( -1 == nread )
		{
			if( EINTR == errno )
				goto read_lable;
			else
				ERR_EXIT( "read error" );
		}

		printf( "Recv from server : " );
		fflush( stdout );
write_label2:
		nwrite = write( 1 , buf , nread );
		if( 0 == nwrite )
			break;
		if( -1 == nwrite )
		{
			if( EINTR == errno )
				goto write_label2;
			else
				ERR_EXIT( "write error" );
		}
	}
	close( sfd );

}// do_service	
