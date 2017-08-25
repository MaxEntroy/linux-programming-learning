#include <sys/types.h>	// for socket setsockopt
#include <sys/socket.h>	// for socket
#include <netinet/in.h> // for struct sockaddr_in htonl htons
#include <arpa/inet.h>  // for inet_addr

#include <sys/select.h> // for select
#include <sys/time.h>	// for struct timeval
#include <unistd.h>		// for select
#include <errno.h>

#include <stdio.h>		// for perror
#include <stdlib.h>		// for exit
#include <stdint.h>		// for uint16_t
#include <strings.h>	// for bzero

#define ERR_EXIT( msg )\
	do{\
		perror(msg);\
		exit( EXIT_FAILURE );\
	}while(0)

#define ERR_EXIT1( msg )\
	do{\
		fprintf( stderr , "%s\n" , msg);\
		exit( EXIT_FAILURE );\
	}while(0)

#define IP_ADDRESS "192.168.4.20"
#define PORT 9734
#define LISTEN_BACKLOG 5
#define MAXLEN 128

static int socket_bind( const char* ip_address , const uint16_t port );
static void socket_listen( int listen_sfd , int backlog );
static void do_select( int listen_sfd );
static void handle_connection( int* client_sfd_arr , int client_num , struct sockaddr_in* client_addr_arr , fd_set* preadfds , fd_set* pallfds );

int main( void )
{
	int listen_sfd = 0;

	listen_sfd = socket_bind( IP_ADDRESS , PORT );
	socket_listen( listen_sfd , LISTEN_BACKLOG );
	do_select( listen_sfd );

	exit( EXIT_SUCCESS );

}// main

static int socket_bind( const char* ip_address , const uint16_t port )
{
	int listen_sfd = 0;
	struct sockaddr_in server_addr;
	socklen_t server_len = 0;

	int ret = 0;

	int optval = 1;
	socklen_t optlen = sizeof( optval );

	// check the argument
	if( NULL == ip_address || port < 0 )
		ERR_EXIT1( "Argument to socket_bind is not valid!" );

	// create a socket
	listen_sfd = socket( AF_INET , SOCK_STREAM , 0 );
	if( -1 == listen_sfd ) 
		ERR_EXIT( "socket error" );

	// set the server address
	bzero( &server_addr , sizeof( server_addr ) );
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( PORT );
	server_addr.sin_addr.s_addr = inet_addr( IP_ADDRESS );

	// set reuse address and port
	ret = setsockopt( listen_sfd , SOL_SOCKET , SO_REUSEADDR , &optval , optlen );
	if( -1 == ret )
		ERR_EXIT( "setsockopt error" );

#ifdef SO_REUSEPORT
	ret = setsockopt( listen_sfd , SOL_SOCKET , SO_REUSEPORT , &optval , optlen );
	if( -1 == ret )
		ERR_EXIT( "setsockopt error" );
#else
	fprintf( stderr , "SO_REUSEPORT is not supported!\n" );
#endif

	// assign a name to socket 
	server_len = (socklen_t)sizeof( server_addr );
	ret = bind( listen_sfd , (struct sockaddr*)&server_addr , server_len );
	if( -1 == ret )
		ERR_EXIT( "bind error" );

	return listen_sfd;

}// socket_bind

static void socket_listen( int listen_sfd , int backlog )
{
	int ret = 0;

	// check the argument
	if( listen_sfd < 0 || backlog <= 0 )
		ERR_EXIT1( "Argument to socket_listen is not valid!" );

	ret = listen( listen_sfd , backlog );
	if( -1 == ret )
		ERR_EXIT( "listen error" );

}// socket_listen

static void do_select( int listen_sfd )
{
	fd_set readfds , allfds;
	struct timeval timeout;

	int client_sfd = 0;
	struct sockaddr_in client_addr; 
	socklen_t client_len = 0;

	int client_sfd_arr[ FD_SETSIZE ];
	int client_num = 0;
	struct sockaddr_in client_addr_arr[ FD_SETSIZE ];

	int max_sfd = listen_sfd;
	
	int ret = 0;
	int nready = 0;
	int i = 0;

	// check the argument
	if( listen_sfd < 0 )
		ERR_EXIT1( "Argument to do_select is not valid!" );

	FD_ZERO( &allfds );
	FD_SET( listen_sfd , &allfds );

	for( i = 0 ; i != FD_SETSIZE ; ++i )
	{
		client_sfd_arr[ i ] = -1;
	}
	bzero( client_addr_arr , sizeof( client_addr_arr ) );

	printf( "server waiting...\n" );
	while(1)
	{
		readfds = allfds;
		timeout.tv_sec = 2;
		timeout.tv_usec =500000;
	
		nready = select( max_sfd + 1 , &readfds , NULL , NULL , &timeout );
		if( -1 == nready )
		{
			if( EINTR == errno )
				continue;
			else
				ERR_EXIT( "select" );
		}
		if( 0 == nready )
		{
			printf( "timeout!\n" );
			continue;
		}

		// handle listen_sfd
		if( FD_ISSET( listen_sfd , &readfds ) )
		{
			client_len = sizeof( client_addr );
			client_sfd = accept( listen_sfd , (struct sockaddr*)&client_addr , &client_len );
			if( -1 == client_sfd )
			{
				if( EINTR == errno )
					continue;
				else
					ERR_EXIT( "accept error" );
			}
			
			// show connect info
			printf( "%s %d connected to server.\n" , inet_ntoa( client_addr.sin_addr ) , client_addr.sin_port );

			// 将client_sfd添加到客户扫描队列
			for( i = 0 ; i != FD_SETSIZE ; ++i )
			{
				if( -1 == client_sfd_arr[i] )
				{
					client_sfd_arr[ i ] = client_sfd;
					client_addr_arr[ i ] = client_addr;
					break;
				}
			}
			if( FD_SETSIZE == i )
				ERR_EXIT1( "Too many clients!" );

			// 更新最大用户数 - 并不是实际的用户数
			client_num = ( (i + 1) > client_num ) ? (i + 1) : client_num;

			// 将client_sfd添加到监听集合
			FD_SET( client_sfd , &allfds );

			// 更新最大监听数
			max_sfd = ( client_sfd > max_sfd ) ? client_sfd : max_sfd;

			// 优化
			if( --nready <= 0 )
				continue;
		}

		// handle conn_sfd - 扫描客户队列
		handle_connection( client_sfd_arr , client_num , client_addr_arr ,&readfds , &allfds );
	}

}// do_select
static void handle_connection( int* client_sfd_arr , int client_num , struct sockaddr_in* client_addr_arr , fd_set* preadfds , fd_set* pallfds )
{
	int i = 0;
	int nread = 0;
	int nwrite = 0;

	char buf[MAXLEN];

	// check the argument
	if( NULL == client_sfd_arr || 0 == client_num || NULL == client_addr_arr || NULL == preadfds || NULL == pallfds )
		ERR_EXIT1( "Argument to handle_connection is not valid!" );

	for( i = 0 ; i != client_num ; ++i )
	{
		if( FD_ISSET( client_sfd_arr[i] , preadfds ) )
		{
			// read from client
			bzero( buf , sizeof(buf) );
			nread = read( client_sfd_arr[i] , buf , MAXLEN );
			if( -1 == nread )
			{
				if( EINTR == errno )		// 这么写的依据还有待于进一步考察 像select这么写是完全没有问题的 其他都需要考察
					continue;
				else
					ERR_EXIT( "read error" );
			}
			// 通信结束 删除当前客户
			if( 0 == nread )
			{
				close( client_sfd_arr[i] );
				FD_CLR( client_sfd_arr[i] , pallfds );
				client_sfd_arr[i] = -1;
				
				// show disconnect info
				printf( "%s %d disconnected to server.\n" , inet_ntoa( client_addr_arr[i].sin_addr ) , client_addr_arr[i].sin_port );

				continue;
			}

			// write to client
			nwrite = write( client_sfd_arr[i] , buf , nread );
			if( -1 == nwrite )
			{
				if( EINTR == errno )
					continue;
				else
					ERR_EXIT( "write error" );
			}
		}
	}

}// handle_connection
