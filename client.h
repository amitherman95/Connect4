
#ifndef _CLIENT_H
#define _CLIENT_H
#include "socket.h"
#include "messages.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>




#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )


#define HUMAN_MODE 0
#define FILE_MODE 1

#define SERVER_ADDRESS_STR "127.0.0.1"
#define NEW_USER_ACCEPTED 0
#define NEW_USER_DECLINED 1
#define GAME_STARTED 2
#define BOARD_VIEW 3
#define TURN_SWITCH 4
#define PLAY_ACCEPTED 5
#define PLAY_DECLINED 6
#define GAME_ENDED 7
#define RECEIVE_MESSAGE 8
// client to server
#define NEW_USER_REQUEST 0
#define PLAY_REQUEST 1
#define SEND_MESSAGE 2
//mutex

#define MAX_MESSAGE_TYPE_SIZE 19
/* Parameters for CreateEvent */

static const LPSECURITY_ATTRIBUTES P_SECURITY_ATTRIBUTES = NULL;
static const BOOL IS_MANUAL_RESET = TRUE; /* Manual-reset event */
static const BOOL IS_INITIALLY_SET = FALSE;
//board defines
#define RED_PLAYER 1
#define YELLOW_PLAYER 2
#define BOARD_HEIGHT 6
#define BOARD_WIDTH  7
#define BLACK  15
#define RED    204
#define YELLOW 238
void PrintBoard(int board[][BOARD_WIDTH], HANDLE consoleHandle);
void update_board(char* str);
int exec_protocol(message* msg, SOCKET sender);
void mainClient(int mode, char* inputfile, char* logfile, int port);
char* encryption(char* user_input, int mode);
void insert_substring(char* a, char*, int);
char* substring(char* string, int, int);
message* process_message(const char * message_text);
void remove_all_chars(char* str, char c);
int get_message_code(const char* message_type);
DWORD RecvDataThread(void);
DWORD SendDataThread(void);
DWORD WINAPI user_interface(int inputmode);
HANDLE create_event_simple(void);

#pragma once
#endif