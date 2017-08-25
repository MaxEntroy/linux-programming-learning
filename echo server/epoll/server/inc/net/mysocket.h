#ifndef GUARD_mysocket_h
#define GUARD_mysocket_h
#include <stdint.h> // for uint16_t

int socket_bind( const char* ip_addr , const uint16_t port );
void socket_listen( int listen_sfd , int backlog );

#endif
