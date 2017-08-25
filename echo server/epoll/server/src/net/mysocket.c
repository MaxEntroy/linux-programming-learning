#include <sys/types.h>	// for socket
#include <sys/socket.h>	// for socket setsockopt
#include <netinet/in.h>	// for struct sockaddr_in
#include <arpa/inet.h>	// for inet_addr htons
#include <strings.h>	// for bzero
#include "mysocket.h"
#include "myerr.h"
#include "myconfig.h"

int socket_bind( const char* ip_addr , uint16_t port )
{
	int listen_sfd = 0;
	struct sockaddr_in server_addr;
	socklen_t server_len = 0;

	int optval = 1;
	socklen_t optlen = sizeof(optval);

	int ret = 0;

	// create a socekt
	listen_sfd = socket( AF_INET , SOCK_STREAM , 0 );
	if( -1 == listen_sfd )
		ERR_EXIT( "socket error" );

	// set the address
	bzero( &server_addr , sizeof( server_addr ) );
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( port );
	server_addr.sin_addr.s_addr = inet_addr( ip_addr );

	// set reuse
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
	server_len = sizeof( server_addr );
	ret = bind( listen_sfd , (struct sockaddr*)&server_addr , server_len );
	if( -1 == ret )
		ERR_EXIT( "bind error" );

	return listen_sfd;
	
}// socket_bind

void socket_listen( int listen_sfd , int backlog )
{
	int ret = 0;
	ret = listen( listen_sfd , backlog );
	if( -1 == ret )
		ERR_EXIT( "listen error" );

}// socket_listen


