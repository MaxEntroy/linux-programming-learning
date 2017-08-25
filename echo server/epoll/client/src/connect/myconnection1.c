#include "myconnection.h"
#include "myerr.h"

#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>

#define MAX_LEN 128
#define MAX( a , b ) ( a > b ) ? a : b

void do_connection1( int conn_sfd )
{
	fd_set readfds , allfds;
	int max_sfd = 0;
	int nready = 0;
	int fd = 0;

	char buf[MAX_LEN];
	int nread = 0;
	int nwrite = 0;
	
	int finish = 1;

	FD_ZERO( &readfds );
	FD_ZERO( &allfds );

	// add sfd to the set
	FD_SET( 0 , &allfds );
	FD_SET( conn_sfd , &allfds );
	
	max_sfd = ( STDIN_FILENO , conn_sfd );
	
	while(1)
	{
		if( 1 == finish )
		{
			printf( "Please input a string : " );
			fflush( stdout );
		}
		
		readfds = allfds;

		nready = select( max_sfd + 1 , &readfds , NULL , NULL , NULL );
		
		if( -1 == nready )
		{
			if( EINTR == errno )
				continue;
			else
				ERR_EXIT( "select error" );
		}
		
		for( fd = 0 ; fd <= max_sfd ; ++fd )
		{
			if( FD_ISSET( fd , &readfds ) )
			{
				if( STDIN_FILENO == fd )
				{
					bzero( buf , sizeof(buf) );
					nread = read( fd , buf , MAX_LEN );
					if( -1 == nread )
					{
						if( EINTR == errno )
						{// no - op
						}
						else
							ERR_EXIT( "read error" );
					}
					else if( 0 == nread )
						goto label;
					else
					{
						// write to server
						nwrite = write( conn_sfd , buf , nread );
						if( -1 == nwrite )
						{
							if( EINTR == errno )
							{	// no - op
							}
							else
								ERR_EXIT( "write error" );
						}
						else if( 0 == nwrite )
							ERR_EXIT1( "write to server error!" );
					}

					finish = 0;
				}
				else
				{
					// read from server
					bzero( buf , sizeof( buf ) );
					nread = read( fd , buf , MAX_LEN );
					if( -1 == nread )
					{
						if( EINTR == errno )
						{	// no - op
						}
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
						{	// no - op
						}
						else
							ERR_EXIT( "write error" );
					}
					else if ( 0 == nwrite )
						ERR_EXIT1( "write to stdout error!" );
					
					finish = 1;
				}
			}
			else
			{}
		}//for
	}// while

label:
	close( conn_sfd );
}
