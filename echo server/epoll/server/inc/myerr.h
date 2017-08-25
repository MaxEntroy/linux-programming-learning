#ifndef GUARD_myerr_h
#define GUARD_myerr_h

#include <stdio.h>	// for perror fprintf
#include <stdlib.h>	// for exit

#define ERR_EXIT(msg)\
	do{\
		perror(msg);\
		exit(EXIT_FAILURE);\
	}while(0)

#define ERR_EXIT1(msg)\
	do{\
		fprintf( stderr , "%s\n" , msg );\
		exit(EXIT_FAILURE);\
	}while(0)

#endif
