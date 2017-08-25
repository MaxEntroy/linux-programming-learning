#ifndef GUARD_myconnection_h
#define GUARD_myconnection_h

void do_connection( int sfd );
void do_connection1( int conn_sfd );// select
void do_connection2( int conn_sfd );// poll
void do_connection3( int conn_sfd );// epoll

#endif
