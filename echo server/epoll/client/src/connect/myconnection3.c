#include "myerr.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

#define MAX_CLIENT 1024
#define MAX_LEN 128
#define INFTIM -1

void do_connection3( int conn_sfd )
{
	int epfd = 0;
	struct epoll_event ev;
	struct epoll_event ev_set[MAX_CLIENT];

	int nready = 0;
	int ret = 0;
	int finish = 1;
	int i = 0;
	int sfd = 0;
	int nread = 0;
	int nwrite = 0;

	char buf[MAX_LEN];

	if( conn_sfd < 0 )
		ERR_EXIT1( "Argument to do_connection3 is not valid!" );

	// create an epoll instance
	epfd = epoll_create( MAX_CLIENT );
	if( -1 == epfd )
		ERR_EXIT( "epoll_create error" );

	// add conn_sfd and STDIN_FILENO to epoll instance
	bzero( &ev , sizeof(ev) );
	ev.data.fd = STDIN_FILENO;
	ev.events = EPOLLIN;
	
	ret = epoll_ctl( epfd , EPOLL_CTL_ADD , STDIN_FILENO , &ev );
	if( -1 == ret )
		ERR_EXIT( "epoll_ctl error" );

	bzero( &ev , sizeof(ev) );
	ev.data.fd = conn_sfd;
	ev.events = EPOLLIN;

	ret = epoll_ctl( epfd , EPOLL_CTL_ADD , conn_sfd , &ev );
	if( -1 == ret )
		ERR_EXIT( "epoll_ctl error" );

	while(1)
	{
		if( 1 == finish )
		{
			printf( "Please input a string : " );
			fflush( stdout );
		}
		
		// epoll wait
		bzero( ev_set , sizeof( ev_set ) );
		nready = epoll_wait( epfd , ev_set , MAX_CLIENT , INFTIM );
		if( -1 == nready )
		{
			if( EINTR == errno )
				continue;
			else
				ERR_EXIT( "epoll_wait error" );
		}

		for( i = 0 ; i != nready ; ++i )
		{
			sfd = ev_set[i].data.fd;

			if( STDIN_FILENO == sfd && ( ev_set[i].events&EPOLLIN ) ) // STDIN_FINENO
			{
				bzero( buf , sizeof(buf) );
				nread = read( STDIN_FILENO , buf , MAX_LEN );
				if( -1 == nread )
				{
					if( EINTR == errno )
					{}// no - op
					else
						ERR_EXIT( "read error" );
				}
				else if( 0 == nread )
					goto label;

				// write to server
				nwrite = write( conn_sfd , buf , nread );
				if( -1 == nwrite )
				{
					if( EINTR == errno )
					{}// no - op
					else
						ERR_EXIT( "write error" );
				}
				else if( 0 == nwrite )
					ERR_EXIT1( "write to server error!" );

				finish = 0;
			}
			else if( ev_set[i].events&EPOLLIN )  // conn_sfd
			{
				// read from server
				bzero( buf , sizeof(buf) );
				nread = read( conn_sfd , buf , MAX_LEN );
				if( -1 == nread )
				{
					if( EINTR == errno )
					{}// no - op
					else
						ERR_EXIT( "read error" );
				}
				else if( 0 == nread )
					ERR_EXIT1( "read from server error!" );
				
				printf( "Recv from server : " );
				fflush( stdout );

				nwrite = write( STDOUT_FILENO , buf , nread );
				if( -1 == nwrite )
				{
					if( EINTR == errno )
					{}// no - op
					else
						ERR_EXIT( "write error" );
				}
				else if( 0 == nwrite )
					ERR_EXIT1( "write to stdout error" );

				finish = 1;
			}
		} 
	}

label:
	close( conn_sfd );

}// do_connection3
