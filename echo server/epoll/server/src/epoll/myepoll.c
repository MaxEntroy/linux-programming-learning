#include <sys/epoll.h>  // for epoll
#include <strings.h>	// for bzero
#include <errno.h>		// for errno
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "myepoll.h"
#include "myerr.h"
#include "myconfig.h"

#define MAX_LEN 128

static struct epoll_event ev_arr[MAX_CLIENT];
static struct sockaddr_in client_addr_arr[MAX_CLIENT];

void do_epoll( int listen_sfd )
{
	int epfd = 0;
	int nready = 0;
	int timeout = 0;

	if( listen_sfd < 0 )
		ERR_EXIT1( "Argument to do_epoll is not valid!" );

	// create an epoll instance
	epfd = epoll_create( MAX_CLIENT );
	if( -1 == epfd )
		ERR_EXIT( "epoll_create error" );

	// add listen sfd to epoll instance
	add_event( epfd , listen_sfd , EPOLLIN );
	
	bzero( &client_addr_arr , sizeof( client_addr_arr ) );
	timeout = 2500;
	printf( "server waiting...\n" );
	while(1)
	{
		nready = epoll_wait( epfd , ev_arr , MAX_CLIENT , timeout );		
		if( -1 == nready )
		{
			if( EINTR == errno )
				continue;
			else
				ERR_EXIT( "epoll_wait error" );
		}
		else if( 0 == nready )
		{
			printf( "timeout!\n" );
			continue;
		}
		else
		{
			handle_events( epfd , ev_arr , nready , listen_sfd , client_addr_arr );
		}
	}

}

void handle_events( int epfd , struct epoll_event* ev_arr , int ev_num , int listen_sfd , struct sockaddr_in* client_addr_arr )
{
	int i = 0;
	int sfd = 0;

	int client_sfd = 0;
	struct sockaddr_in client_addr;
	socklen_t client_len = 0;

	int nread = 0;
	int nwrite = 0;
	char buf[MAX_LEN];

	if( epfd < 0 || NULL == ev_arr || ev_num < 0 || listen_sfd < 0 || NULL == client_addr_arr )
		ERR_EXIT1( "Argument to handle_events is not valid!" );

	for( i = 0 ; i != ev_num ; ++i )
	{
		sfd = ev_arr[i].data.fd;
		
		if( sfd == listen_sfd && ( ev_arr[i].events & EPOLLIN ) )
		{
			bzero( &client_addr , sizeof( client_addr ) );
			client_len = sizeof( client_addr );
			
			client_sfd = accept( listen_sfd , (struct sockaddr*)&client_addr , &client_len );			
			if( -1 == client_sfd )
				ERR_EXIT( "accept error" );
			
			// add the client_sfd to epoll instance
			add_event( epfd , client_sfd , EPOLLIN );
			
			client_addr_arr[ client_sfd ] = client_addr;
			
			// show the connect info
			printf( "%s %d connect to server.\n" , inet_ntoa( client_addr.sin_addr ) , client_addr.sin_port );

		}
		else if( ev_arr[i].events & EPOLLIN )
		{
			// read from client
			bzero( buf , sizeof( buf ) );
			nread = read( sfd , buf , MAX_LEN );
			
			if( -1 == nread )
			{
				if( EINTR == errno )	// 这里的写法有待商榷
					continue;
				else
					ERR_EXIT( "read error" );
			}
			else if ( 0 == nread )
			{
				// delete the sfd from epoll instance
				del_event( epfd , sfd , EPOLLIN );
				close( sfd );
				
				// show the disconnet info
				printf( "%s %d disconnect to server.\n" , inet_ntoa( client_addr_arr[sfd].sin_addr ) , client_addr_arr[sfd].sin_port );

			}
			else
			{
				// write to client
				nwrite = write( sfd , buf , nread );
				if( -1 == nwrite )
				{
					if( EINTR == errno )
						continue;		// 这里的写法有待商榷
					else
						ERR_EXIT( "write error" );
				}
			}
		}	
	}// for

}// handle_events

void add_event( int epfd , int sfd , int events )
{
	struct epoll_event ev;
	int ret = 0;

	if( epfd < 0 || sfd < 0 || events < 0 )
		ERR_EXIT1( "Argument to add_event is not valid!" );

	bzero( &ev , sizeof( ev ) );
	ev.data.fd = sfd;
	ev.events = events;

	ret = epoll_ctl( epfd , EPOLL_CTL_ADD , sfd , &ev );
	if( -1 == ret )
		ERR_EXIT( "epoll_ctl add error" );

}// add_event

void mod_event( int epfd , int sfd , int events )
{
	struct epoll_event ev;
	int ret = 0;

	if( epfd < 0 || sfd < 0 || events < 0 )
		ERR_EXIT1( "Argument to mod_event is not valid!" );


	bzero( &ev , sizeof( ev ) );
	ev.data.fd = sfd;
	ev.events = events;

	ret = epoll_ctl( epfd , EPOLL_CTL_MOD , sfd , &ev );
	if( -1 == ret )
		ERR_EXIT( "epoll_ctl add error" );

}// mod_event

void del_event( int epfd , int sfd , int events )
{
	struct epoll_event ev;
	int ret = 0;

	if( epfd < 0 || sfd < 0 || events < 0 )
		ERR_EXIT1( "Argument to del_event is not valid!" );
	
	bzero( &ev , sizeof( ev ) );
	ev.data.fd = sfd;
	ev.events = events;

	ret = epoll_ctl( epfd , EPOLL_CTL_DEL , sfd , &ev );
	if( -1 == ret )
		ERR_EXIT( "epoll_ctl add error" );

}// del_event

