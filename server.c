/*
Name:server.c
Description: Server routines library
Amit Herman Raz Rajwan
*/


#define _CRT_SECURE_NO_WARNINGS
#include "server.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "threads.h"
#include "game.h"
#include "socket.h"
#include "messages.h"


/*	Constants		*/
#define NUM_OF_WORKER_THREADS 2
#define MAX_LOOPS 2
#define SEND_STR_SIZE 35
#define NEW_USER_REQUEST 0
#define PLAY_REQUEST 1
#define SEND_MESSAGE 2
#define MAX_MESSAGE_TYPE_SIZE 19

/*	MACROS	*/
#define DIGIT2ASCII(X) (X)+48
#define GET_OTHER_SOCKET_NUM(X) ((X) == 0) ? (1) : (0))

/*	Globals	*/
HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET Sockets[NUM_OF_WORKER_THREADS] = { INVALID_SOCKET ,INVALID_SOCKET };
static HANDLE h_mutex = NULL;
int board[6][7];
const char * server_messages[] = { "NEW_USER_REQUEST", "PLAY_REQUEST","SEND_MESSAGE" }; //extern because client.c uses this too

/*	Utility functions that support other function for general things	*/
int get_message_code_server(const char*message_type) {
	int i;

	for (i = 0; i < 3; i++) {
		if (strstr(message_type, server_messages[i]) == message_type)
			return i;

	}
	return -1;
}

//This function divides the message string to the type and parameters and places them in a special structure
//Input: The received string from the socket
//Output: message structure
message* process_Message(const char* message_text) {
	char message_type_string[MAX_MESSAGE_TYPE_SIZE];
	int message_type_size;
	char *token;
	int message_type_code;
	char *temp = 0;
	char * lp_params = NULL;
	message * proccessed_message;
	proccessed_message = (message*)malloc(sizeof(message));

	/* get the message type */
	if ((lp_params = strchr(message_text, ':')) == NULL)//parameterless message type
		message_type_code = get_message_code_server(message_text);

	else {
		message_type_size = (int)(lp_params - message_text);//lp_params-message=length of the first part of the message, e.g.:SEND_MESSAGE ==> 11 chars
		memcpy(message_type_string, message_text, message_type_size);//extract the first time of the message
		message_type_string[message_type_size] = '\0'; //add null terminator
		message_type_code = get_message_code_server(message_type_string);
	}

	if (message_type_code >= 0)
	{
		temp = (lp_params)+1;

		proccessed_message->message_arguments = temp;
	}
	else
	{
		proccessed_message->message_arguments = NULL;
	}

	proccessed_message->message_type = message_type_code;

	return proccessed_message;


}
void copy_parameters(char*target, char*param) {
	/*just copies string from one to another without chars of type ';'*/

	int i, k;

	k = 0;
	for (i = 0; i < strlen(param); i++) {
		if (param[i] != ';') {
			target[k] = param[i];
			k++;
		}

		target[k] = '\0';
	}

}
void close_sockets(SOCKET socks[]) {

	int i;
	for (i = 0; i < NUM_OF_WORKER_THREADS; i++) {
		if (socks[i] != INVALID_SOCKET) {
			if (closesocket(socks[i])== SOCKET_ERROR)
				printf("Error closing:%ld\n", WSAGetLastError());
		}
	}
}

/*	Functions that sends messages to one or both clients(excluding arbitrary chat messages)	*/
int send_game_started(SOCKET sender, SOCKET other_player) {
	TransferResult_t SendRes;
	char SendStr[] = "GAME_STARTED";

	SendRes = SendString(SendStr, sender);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(sender);
		return 0;
	}

	SendRes = SendString(SendStr, other_player);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(other_player);
		return 0;
	}
	return 1;
}
int send_new_user_accepted(SOCKET sender) {
	TransferResult_t SendRes;
	char SendStr[20];
	strcpy(SendStr, "NEW_USER_ACCEPTED:");
	SendStr[18] = get_players_num_ready() + 48;
	SendStr[19] = '\0';
	SendRes = SendString(SendStr, sender);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(sender);
		return 0;
	}
	return 1;
}
int send_new_user_declined(SOCKET sender) {
	TransferResult_t SendRes;
	char SendStr[20];
	strcpy(SendStr, "NEW_USER_DECLINED");
	SendRes = SendString(SendStr, sender);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(sender);
		return 0;
	}
	return 1;
}
int send_board_view(SOCKET sender, SOCKET other_player) {
	TransferResult_t SendRes;
	int i,j,k;
	char SendStr[11 + 42 + 41 + 1];

	strcpy(SendStr, "BOARD_VIEW:");
	k = 0;
	for (i = 0; i < 6; i++)
		for (j = 0; j < 7; j++) {
			SendStr[11 + k] = DIGIT2ASCII(board[i][j]);
			SendStr[11 + k + 1] = ';';
			k = k + 2;
		}
	SendStr[11 + k - 1] = '\0';
			
	SendRes = SendString(SendStr, sender);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(sender);
		return 0;
	}


	SendRes = SendString(SendStr, other_player);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(other_player);
		return 0;
	}
	return 1;
}
int send_play_decline(SOCKET sender, const char * error_message){

	TransferResult_t SendRes;
	char *SendStr=NULL;

	SendStr = (char*)malloc(13 + strlen(error_message) + 1);
	if (SendStr == NULL) {
		printf("Allocation error\n");
		return 0;
	}

	strcpy(SendStr, "PLAY_DECLINED:");
	strcat(SendStr, error_message);
	SendRes = SendString(SendStr, sender);
	free(SendStr);
	if (SendRes == TRNS_FAILED) {
		printf("Service socket error\n");
		closesocket(sender);
		return 0;
	}


	return 1;
}
int send_turn_switch(SOCKET sender, SOCKET other_player) {

	TransferResult_t SendRes;
	char SendStr[13 + 30]; // 13 chars for "TURN_SWITCH", 30 chars for the name
	strcpy(SendStr, "TURN_SWITCH:");
	strcat(SendStr, get_player_name(whose_turn()-1)); // we decrease by one since whose_turn() returns 1 or 2,
																										//and we need 0 or 1 as inputs for get_player_name()
	SendRes = SendString(SendStr, sender);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(sender);
		return 0;
	}

	SendRes = SendString(SendStr, other_player);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(other_player);
		return 0;
	}

	return 1;

}
int send_play_accepted(SOCKET sender) {
	TransferResult_t SendRes;
	char SendStr[20];
	strcpy(SendStr, "PLAY_ACCEPTED");
	SendRes = SendString(SendStr, sender);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(sender);
		return 0;
	}
	return 1;

}
int send_game_ended(SOCKET sender, SOCKET other_player, int winner_or_tie) {

	TransferResult_t SendRes;
	char SendStr[12 + 30+1]; // 12 chars for "TURN_SWITCH", 30 chars for the name

	strcpy(SendStr, "GAME_ENDED:");
	if (winner_or_tie == TIE)
			strcat(SendStr, "Tie"); else
				strcat(SendStr, get_player_name(winner_or_tie - 1)); // we decrease by one since whose_turn() returns 1 or 2,		
																														 //and we need 0 or 1 as inputs for get_player_name()
	SendRes = SendString(SendStr, sender);
	if (SendRes == TRNS_FAILED){
		printf("Service socket error\n");
		closesocket(sender);
		return 0;
	}

	SendRes = SendString(SendStr, other_player);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(other_player);
		return 0;
	}

	return 1;

}


/*	Functions that handle particular incoming messages	*/
int send_message_server(message*lp_message, SOCKET s_target,int sock_num) {

	TransferResult_t SendRes;
	char* SendStr = NULL;

	SendStr = (char*)malloc(1 + strlen(lp_message->message_arguments) + strlen("RECEIVE_MESSAGE:")+30);
	if (SendStr == NULL) {
		printf("Allocation error\n");
		return 0;

	}
	strcpy(SendStr, "RECEIVE_MESSAGE:");
	strcat(SendStr, get_player_name(sock_num));
	strcat(SendStr, ";");
	strcat(SendStr, lp_message->message_arguments);
	SendRes = SendString(SendStr, s_target);
	free(SendStr);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(s_target);
		return 0;
	}

	
	return 1;
}
int play_request(message* msg, SOCKET sender, int socket_num, SOCKET other) {

	int player;
	int column;

	int winner;
	/*	the first socket belong to the red player
			the second socket belong to the yellow player		*/
	switch (socket_num) {
	case 0:
		player = RED_PLAYER;
		break;
	case 1:
		player = YELLOW_PLAYER;
		break;
	}

	if (get_status()==WAITING) {
		if (!send_play_decline(sender, "Game has not started"))
			return 0; else
				return 1;
	}

	if (player != whose_turn()) {
		if (!send_play_decline(sender, "Not your turn"))
			return 0; else
				return 1;
	}

	sscanf(msg->message_arguments, "%d", &column);
	if (column < 0 || column>6) {
		if (!send_play_decline(sender, "Illegal move"))
			return 0; else
				return 1;
	}

	/*	Legal request, now check if the move is legal	*/
	if (!insert_disc(column, player, board)) {
		if (!send_play_decline(sender, "Illegal move"))
			return 0; else
			return 1;
	}

	if (!send_play_accepted(sender))
		return 0;
	if (!send_board_view(sender, other))
		return 0;

	/*Check for a winner*/
	winner = check_win(board);
	if (winner != 0) {
		if (!send_game_ended(sender, other, winner))
			return 0;
		switch (winner) {
		case YELLOW_PLAYER:
			set_status(YELLOW_WIN);
			break;
		case RED_PLAYER:
			set_status(RED_WIN);
			break;


	
		}
		return 1;
	}

	/*check for tie*/
	if (check_tie(board)) {
		set_status(TIE);
		return 1;
	}

	switch_turn();
	if (!send_turn_switch(sender, other))
		return 0;

	return 1;
}
int new_user_request(const char *name, SOCKET sender, int socket_num, SOCKET other) {

	DWORD wait_res;
	DWORD release_res;
	TransferResult_t SendRes;
	char SendStr[20];

	/*Already accepted a name, if the length is greater than zero, the player already got username*/
	if (strlen(get_player_name(socket_num)) > 0) {
		if (!send_new_user_declined(sender))
			return 0;
		return 1;
	}
	//Name already taken
	else if (is_name_unavail(name) == TRUE) {
		if (!send_new_user_declined(sender))
			return 0;
		return 1;
	}

	set_player_name(name, socket_num);
	//Reconsider this
	/*Wait*/
	wait_res = WaitForSingleObject(h_mutex, INFINITE);
	if (wait_res != WAIT_OBJECT_0) {
		printf("Mutex wait error\n");
		return 0;
	}

	/*Criticial region*/
	player_ready();
	/*Send message to client*/
	if (!send_new_user_accepted(sender))
		return 0;
	/*Change game status to ready if both clients ready*/
	if (get_players_num_ready() == 2) {
		set_status(READY);
		if (!send_game_started(sender, other))
			return 0;
		if (!send_board_view(sender, other))
			return 0;
		if (!send_turn_switch(sender, other))
			return 0;
	}

	/*Release*/
	release_res = ReleaseMutex(h_mutex);
	if (release_res == FALSE) {
		printf("Mutex release error\n");
		return 0;
	}

	return 1;
}

/*
*	This function executes the proper function that handles the message
*/
int exec_protocol_server(message* lp_message, SOCKET sender, SOCKET other_player, int socket_num) {

	switch (lp_message->message_type) {
	case SEND_MESSAGE:
		return (send_message_server(lp_message, other_player, socket_num));
	case NEW_USER_REQUEST:
		return (new_user_request(lp_message->message_arguments, sender, socket_num, other_player));
	case PLAY_REQUEST:
		return play_request(lp_message, sender,socket_num, other_player);
		break;
	}
}


/*
*Thread function
*	The input is the number of the accepted socket,
*	 the first client will have number 0 and the other one will have number 1
*/
DWORD ServiceThread(int sock_num)
{
	char SendStr[SEND_STR_SIZE];
	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	message *lp_message=NULL;
	SOCKET* sender;
	SOCKET* other;
	int game_status;
	sender = &Sockets[sock_num]; 
	if (sock_num == 0) other = &Sockets[1]; else other = &Sockets[0];


	while (!Done)
	{
		char *AcceptedStr = NULL;

		RecvRes = ReceiveString(&AcceptedStr, *sender);

		if (RecvRes == TRNS_FAILED)
		{
			game_status = get_status();
			if (game_status != WAITING && game_status != READY) 
				return 0;
			printf("Service socket error while reading, closing thread.\n");
			closesocket(*sender);
			free(AcceptedStr);
			return 1;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			game_status = get_status();
			if (game_status != WAITING && game_status != READY)
				return 0;
			printf("Connection closed while reading, closing thread.\n");
			closesocket(*sender);
			free(AcceptedStr);
			return 1;
		}
		else {
			lp_message =process_Message(AcceptedStr);
			if (lp_message != NULL) {
				if (exec_protocol_server(lp_message, *sender, *other, sock_num)==0) {
					free(AcceptedStr);
					delete_message(lp_message); //free memory
					return 1;
				}
			}
			delete_message(lp_message);
		}
	}

	return 0;
}


/*Main server function*/
int mainServer(const char*logfile_path, int port )
{
	int ret_val;
	int Loop;
	SOCKET MainSocket = INVALID_SOCKET;
	SOCKET AcceptSocket = INVALID_SOCKET;
	DWORD code1, code2;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	WSADATA wsaData;
	FILE* logfile;
	
	/*open file*/
	logfile = fopen(logfile_path, "w");
	if (logfile == NULL) {
		printf("Error opening the file/n");
		return 0;
	}

	printf("New Connect4 game \n");

	/*Initialize Mutex*/
	h_mutex = CreateMutexSimple(NULL);
	if (h_mutex == NULL) {
		printf("Error at Mutex creation.\n");
		return 0;
	}
	if (!Init_WinSocket(&wsaData))
		return 0;

	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		return 0;
	}

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		return 0;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(port);

	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		return 1;
	}

	/*Reset game*/
	init_game(board);

	printf("Server ready(listening on PORT:%ld)\n", port);
	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto Error;
	}
	for (Loop = 0; Loop < NUM_OF_WORKER_THREADS; Loop++)
		ThreadHandles[Loop] = NULL;
	printf("Waiting for a client to connect...\n");
	for (Loop = 0; Loop < MAX_LOOPS; Loop++)
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			return 0;
		}
		printf("Client Connected.\n");
		Sockets[Loop] = AcceptSocket;
		ThreadHandles[Loop] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)ServiceThread,
			Loop, // we send the value of the counter which indicate which
			0,
			NULL
		);
	}


	printf("Both clients connected\n");
	closesocket(MainSocket);
	WaitThreads(NUM_OF_WORKER_THREADS, ThreadHandles, FALSE); //wait for one of the thread to finish
	GetExitCodeThread(ThreadHandles[0], &code1);
	GetExitCodeThread(ThreadHandles[1], &code2);
	if (code1 == 0 || code2 == 0) {//if one the threads ended well the other should end well too.
		KillThreads(code1, code2, ThreadHandles);
		close_sockets(Sockets);
		Close_Threads(NUM_OF_WORKER_THREADS, ThreadHandles);
		WSACleanup();
		fclose(logfile);
		return 1;
	}

Error:
	KillThreads(code1, code2, ThreadHandles);
	close_sockets(Sockets);
	Close_Threads(NUM_OF_WORKER_THREADS, ThreadHandles);
	WSACleanup();
	fprintf(logfile, "Player disconnected. Ending communication.\n");
	fclose(logfile);
	printf( "Player disconnected. Ending communication.\n");
	return 0;

}
