#include <sys/types.h>	// for socket
#include <sys/socket.h>	// for socket
#include <netinet/in.h> // for struct sockaddr_in
#include <arpa/inet.h>	// for inet_addr inet_ntoa
#include <poll.h>

#include <stdio.h>		// for perror
#include <stdlib.h>		// for exit
#include <stdint.h>		// for uint16_t
#include <errno.h>		// for EINTR
#include <strings.h>	// for bzero

#define IP_ADDRESS "192.168.4.20"
#define PORT 9734
#define LISTEN_BACKLOG 10
#define MAX_LEN 128
#define MAX_CLIENT 1024

#define ERR_EXIT( err_msg )\
	do{\
		perror( err_msg );\
		exit( EXIT_FAILURE );\
	}while(0)

#define ERR_EXIT1( err_msg )\
	do{\
		fprintf( stderr , "%s\n" , err_msg );\
		exit( EXIT_FAILURE );\
	}while(0)

static int socket_bind( const char* ip_address , const uint16_t port );
static void socket_listen( int listen_sfd , int backlog );
static void do_poll( int listen_sfd );
static void handle_connection( struct pollfd* client_fd_arr , int client_num , struct sockaddr_in* client_addr_arr );

int main( void )
{
	int listen_sfd = 0;

	listen_sfd = socket_bind( IP_ADDRESS , PORT );
	socket_listen( listen_sfd , LISTEN_BACKLOG );
	do_poll( listen_sfd );

	exit( EXIT_FAILURE );
}

static int socket_bind( const char* ip_address , const uint16_t port )
{
	int listen_sfd = 0;
	struct sockaddr_in server_addr;
	socklen_t server_len = 0;
	
	int optval = 1;
	socklen_t optlen = (socklen_t)sizeof( optval );

	int ret = 0;


	if( NULL == ip_address || port <= 0 )
		ERR_EXIT1( "Argument to socket_bind is not valid!" );

	// create the socket
	listen_sfd = socket( AF_INET , SOCK_STREAM , 0 );
	if( -1 == listen_sfd )
		ERR_EXIT( "socket error" );

	// set the address
	bzero( &server_addr , sizeof( server_addr ) );
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( port );
	server_addr.sin_addr.s_addr = inet_addr( ip_address );

	// set reuse addr and port
	ret = setsockopt( listen_sfd , SOL_SOCKET , SO_REUSEADDR , &optval , optlen );
	if( -1 == ret )
		ERR_EXIT( "setsockopt error" );

#ifdef SO_REUSEPORT
	ret = setsockopt( listen_sfd , SOL_SOCKET , SO_REUSEPORT , &optval , optlen );
	if( -1 == ret )
		ERR_EXIT( "setsockopt error" );
#else
	ERR_EXIT1( "SO_REUSEPORT is not supported!" );
#endif

	// assigning a name to a socket
	server_len = (socklen_t)sizeof( server_addr );
	ret = bind( listen_sfd , (struct sockaddr*)&server_addr , server_len );
	if( -1 == ret )
		ERR_EXIT( "bind error" );

	return listen_sfd;

}// socket_bind

static void socket_listen( int listen_sfd , int backlog )
{
	int ret = 0;

	if( listen_sfd < 0 || backlog <=0 )
		ERR_EXIT1( "Argument to socket_listen is not valid!" );
	
	ret = listen( listen_sfd , backlog );
	if( -1 == ret )
		ERR_EXIT( "listen error" );

}// socket_listen

static void do_poll( int listen_sfd )
{
	struct pollfd client_fd_arr[ MAX_CLIENT ];
	int timeout = 0;
	int max_sfd = 0;

	int client_sfd = 0;
	struct sockaddr_in client_addr;
	socklen_t client_len = 0;
	struct sockaddr_in client_addr_arr[ MAX_CLIENT ];

	int nready = 0;
	int ret = 0;
	int i = 0;

	if( listen_sfd < 0 )
		ERR_EXIT1( "Argument to do_poll is not valid!" );

	for( i = 0; i != MAX_CLIENT ; ++i ) 
	{
		client_fd_arr[i].fd = -1;	
	}

	client_fd_arr[0].fd = listen_sfd;
	client_fd_arr[0].events = POLLIN;
	
	max_sfd = listen_sfd;
	bzero( client_addr_arr , sizeof( client_addr_arr ) );
	
	printf( "server waiting...\n" );
	while(1)
	{
		timeout = 2500;	// 2.5s
		
		nready = poll( client_fd_arr , max_sfd + 1 , timeout );	
		if( 0 == nready )
		{
			printf( "timeout!\n" );
			continue;
		}
		if( -1 == nready )
		{
			if( EINTR == errno )
				continue;
			else
				ERR_EXIT( "poll error" );
		}

		// handle listen sfd
		if( client_fd_arr[0].revents & POLLIN )
		{	
			client_len = (socklen_t)sizeof( client_addr );
			client_sfd = accept( listen_sfd , ( struct sockaddr* )&client_addr , &client_len );					
			if( -1 == client_sfd )
			{
				if( EINTR == client_sfd )
					continue;
				else
					ERR_EXIT( "accept error" );
			}

			// show the connect info
			printf( "%s %d connected to server.\n" , inet_ntoa( client_addr.sin_addr ) , client_addr.sin_port );

			// add the client sfd to the poll arr
			for( i = 1 ; i != MAX_CLIENT ; ++i )
			{
				if( -1 == client_fd_arr[i].fd )
				{
					client_fd_arr[i].fd = client_sfd;
					client_fd_arr[i].events = POLLIN;
					
					client_addr_arr[i] = client_addr;
					break;
				}
			}
			if( MAX_CLIENT == i )
				ERR_EXIT1( "Too many clients!" );
			
			max_sfd = ( client_sfd > max_sfd )?client_sfd:max_sfd;
			
			if( --nready <= 0 )
				continue;
		}

		
		// handle connection
		handle_connection( client_fd_arr , max_sfd + 1 , client_addr_arr );
	}


}// do_poll

static void handle_connection( struct pollfd* client_fd_arr , int client_num , struct sockaddr_in* client_addr_arr )
{
	int i = 0;
	int nread = 0;
	int nwrite = 0;
	char buf[MAX_LEN];
	
	if( NULL == client_fd_arr || client_num <= 0 )
		ERR_EXIT1( "Argument to handle_connection is not valid!" );

	bzero( buf , sizeof( buf ) );
	for( i = 1 ; i != client_num ; ++i )
	{
		if( client_fd_arr[i].revents & POLLIN )
		{
			// read from client		
			nread = read( client_fd_arr[i].fd , buf , MAX_LEN );
			if( -1 == nread )
			{
				if( EINTR == errno )	// 这里到底怎么写有待商榷 或者还是根本就不会被中断
					continue;
				else
					ERR_EXIT( "read error" );
			}
			if( 0 == nread )			// IO finish
			{
				// show disconnect info
				printf( "%s %d disconnect to server.\n" , inet_ntoa( client_addr_arr[i].sin_addr ) , client_addr_arr[i].sin_port );

				close( client_fd_arr[i].fd );
				client_fd_arr[i].fd = -1;
				
				continue;
			}

			// write to client
			nwrite = write( client_fd_arr[i].fd , buf , nread );
			if( -1 == nwrite )
			{
				if( EINTR == errno )
					continue;
				else
					ERR_EXIT( "write error" );
			}
			if( 0 == nwrite )			// 这里怎么写有待商榷 我是直接跳过本次链接 去处理下一个链接
				continue;
		}
	}

}// handle_connection
