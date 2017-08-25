#include "myconnection.h"
#include "myerr.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LEN 128
void do_connection( int sfd )
{
	char buf[MAX_LEN];
	int nread = 0;
	int nwrite = 0;

	if( sfd < 0 )
		ERR_EXIT1( "Argument to do_connection is not valid!" );

	while( printf( "Please input a string : " ) , fflush( stdout ) , nread = read( STDIN_FILENO , buf , MAX_LEN ) )
	{
		if( -1 == nread )
		{
			if( EINTR == errno )
				continue;
			else
				ERR_EXIT( "read error" );
		}
		else
		{
w_label:
			// write to server
			nwrite = write( sfd , buf , nread );
			if( -1 == nwrite )
			{
				if( EINTR == errno )
					goto w_label;
				else
					ERR_EXIT( "write error" );
			}
			else if( 0 == nwrite )
				ERR_EXIT1( "write to server error!" );
r_label:
			// read from server
			printf( "Recv from server : " );
			fflush( stdout );

			nread = read( sfd , buf , MAX_LEN);
			if( -1 == nread )
			{
				if( EINTR == errno )
					goto r_label;
				else
					ERR_EXIT( "read error" );
			}
			else if( 0 == nread )
				ERR_EXIT1( "read from server error" );
w_label1:
			nwrite = write( STDOUT_FILENO , buf , nread );
			if( -1 == nwrite )
			{
				if( EINTR == errno )
					goto w_label1;
				else
					ERR_EXIT( "write error" );
			}
			else if( 0 == nwrite )
				ERR_EXIT1( "write to stdout error!" );
		
		}
	}

}
