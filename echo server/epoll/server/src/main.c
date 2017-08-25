#include <stdlib.h>	// for exit
#include "myconfig.h"
#include "mysocket.h"
#include "myepoll.h"

int main( void )
{
	int listen_sfd = 0;
	
	listen_sfd = socket_bind( IP_ADDRESS , PORT );
	socket_listen( listen_sfd , LISTEN_BACKLOG );
	
	do_epoll( listen_sfd );

	exit( EXIT_SUCCESS );
}


