#include "myconnection.h"
#include "myerr.h"

#include <errno.h>
#include <sys/poll.h>
#include <unistd.h>
#include <strings.h>

#define MAX_CLIENT 2
#define MAX_LEN 128
#define INFTIM -1

void do_connection2( int conn_sfd )
{
	struct pollfd fd_set[ MAX_CLIENT ];
	int nready = 0;
	int max_sfd = 0;
	
	char buf[ MAX_LEN ];
	int nread = 0;
	int nwrite = 0;
	
	int i = 0;
	int finish = 1;

	if( conn_sfd < 0 )
		ERR_EXIT1( "Argument to do_connection2 is not valid!" );

	// register the fd to poll instance
	bzero( fd_set , sizeof( fd_set ) );

	fd_set[0].fd = STDIN_FILENO;
	fd_set[0].events = POLLIN;

	fd_set[1].fd = conn_sfd;
	fd_set[1].events = POLLIN;

	while(1)
	{
		if( 1 == finish )
		{
			printf( "Please input a string : " );
			fflush( stdout );
		}

		nready = poll( fd_set , MAX_CLIENT , INFTIM );
		if( -1 == nready )
		{
			if( EINTR == errno )
				continue;
			else
				ERR_EXIT( "poll error" );
		}

		for( i = 0 ; i != MAX_CLIENT ; ++i )
		{
			if( fd_set[i].revents & POLLIN )
			{
				if( STDIN_FILENO == fd_set[i].fd ) // STDIN_FILENO
				{
					bzero( buf , sizeof( buf ) );
					nread = read( STDIN_FILENO , buf , MAX_LEN );
					if( -1 == nread  )
					{
						if( EINTR == errno )
						{}// no -op
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
						{}// no -op
						else
							ERR_EXIT( "write error" );
					}
					else if( 0 == nwrite )
						ERR_EXIT1( "write to server error!" );

					finish = 0;
				}
				else // conn_sfd
				{
					// read from server
					bzero( buf , sizeof( buf ) );
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
						ERR_EXIT1( "write to stdout error!" );
					
					finish = 1;
				}
			}
		}
	}

label:
	close( conn_sfd );

}// do_connection2
