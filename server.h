/* 
Name:server.c
Description: Server routines library headers
*/


#ifndef _SERVER_H
#define _SERVER_H

#include "socket.h"

#define SEND_STR_SIZE 50
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
#define SERVER_ADDRESS_STR "127.0.0.1"


/*	Incoming message codes	 */
#define NEW_USER_REQUEST 0x1
#define PLAY_REQUEST 0x2
#define SEND_MESSAGES 0x3

/*Function declarations*/
int mainServer(const char* logfile_path, int port);
DWORD ServiceThread(int sock_num);


typedef struct SockThread_param {
	SOCKET sock;
	char**buffer;
} working_thread_param;


#endif
