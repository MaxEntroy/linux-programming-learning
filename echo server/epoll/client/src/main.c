#include "mysocket.h"
#include "myconnection.h"
#include "myconfig.h"

#include <stdlib.h>

int main( void )
{
	int conn_sfd = 0;

	conn_sfd = socket_connect( IP_ADDRESS , PORT );
	do_connection3( conn_sfd );

	exit( EXIT_FAILURE );
}
