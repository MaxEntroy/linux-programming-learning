#ifndef GUARD_myepoll_h
#define GUARD_myepoll_h
#include <sys/epoll.h>
#include <netinet/in.h>

void do_epoll( int listen_sfd );
void
handle_events(	int epfd , struct epoll_event* ev_arr , int ev_num , 
				int listen_sfd , struct sockaddr_in* client_addr_arr );

void add_event( int epfd , int sfd , int events );
void mod_event( int epfd , int sfd , int events );
void del_event( int epfd , int sfd , int events );

#endif
