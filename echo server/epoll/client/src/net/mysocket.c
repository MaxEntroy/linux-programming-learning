#include "mysocket.h"
#include "myerr.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // for struct sockaddr_in
#include <arpa/inet.h>  // for htons inet_addr
#include <strings.h>	// for bzero

int socket_connect( const char* ip_address , const uint16_t port )
{
	int conn_sfd = 0;
	struct sockaddr_in server_addr;
	socklen_t server_len = 0;

	int ret = 0;

	if( NULL == ip_address || port < 0 )
		ERR_EXIT1( "Argument to socket_connect is not valid!" );

	// create a socket
	conn_sfd = socket( AF_INET , SOCK_STREAM , 0 );
	if( -1 == conn_sfd )
		ERR_EXIT( "socket error" );

	// set the server address
	bzero( &server_addr , sizeof( server_addr ) );
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( port );
	server_addr.sin_addr.s_addr = inet_addr( ip_address );

	// connect to server
	server_len = sizeof( server_addr );
	ret = connect( conn_sfd , (struct sockaddr*)&server_addr , server_len );
	if( -1 == ret )
		ERR_EXIT( "connect error" );

	return conn_sfd;

}// socket_connect
