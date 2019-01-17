#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "client.h"


int board[BOARD_HEIGHT][BOARD_WIDTH] = { 0 };
char * client_Messages[] = { "NEW_USER_ACCEPTED","NEW_USER_DECLINED","GAME_STARTED","BOARD_VIEW","TURN_SWITCH","PLAY_ACCEPTED","PLAY_DECLINED","GAME_ENDED","RECEIVE_MESSAGE" };
char * server_Messages[] = { "NEW_USER_REQUEST","PLAY_REQUEST","SEND_MESSAGE" };
char username[100];
char* global_inputfile;
char* global_logfile;
char SendStr_global[256];
static HANDLE logfile_mutex;
HANDLE Send_event;
HANDLE turn_event;
HANDLE  hConsole = NULL;
SOCKET m_socket;
FILE* logfile_pointer;//mutal FILE pointer of the receiving  and the sending thread. 
/*This thread recieve messages from the server and executes them. */
DWORD RecvDataThread(void)
{
	TransferResult_t RecvRes;
	DWORD wait_res;
	BOOL release_res;
	int exit = 0;
	while (1)
	{
		char *AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, m_socket);
		WaitForSingleObject(logfile_mutex, INFINITE);
		/* Start Critical Section */
		fprintf(logfile_pointer, "Received from server: %s\n", AcceptedStr);
		release_res = ReleaseMutex(logfile_mutex);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Server closed connection. Bye!\n");
			return 0x555;
		}


		message* msg= process_message(AcceptedStr);// processing recieved message
		exit = exec_protocol(msg,m_socket);// executing server commands
		free(AcceptedStr);
		if (exit == 1)// exiting the thread if the game is ended or user declined.(exiting the program too).
			break;
	}
	return 0;
}


DWORD SendDataThread(void)//sending only Thread
{
	BOOL is_success;
	BOOL release_res;
	char SendStr[256];
	TransferResult_t SendRes;
	
	while (1)

	{
		WaitForSingleObject(Send_event, INFINITE);
		strcpy(SendStr, SendStr_global);
		is_success = ResetEvent(Send_event);

		if (STRINGS_ARE_EQUAL(SendStr, "quit"))
			return 0x555; //"quit" signals an exit from the client side
		SendRes = SendString(SendStr, m_socket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		//writing to log file.
		WaitForSingleObject(logfile_mutex, INFINITE);
		/* Start Critical Section */
		
		fprintf(logfile_pointer,"Sent to server: %s\n",SendStr);
		release_res = ReleaseMutex(logfile_mutex);

	}
}
HANDLE create_event_simple()// create event
{
	HANDLE event_handle;
	DWORD last_error;

	/* Get handle to event by name. If the event doesn't exist, create it */
	event_handle = CreateEvent(
		P_SECURITY_ATTRIBUTES, /* default security attributes */
		IS_MANUAL_RESET,       /* manual-reset event */
		IS_INITIALLY_SET,      /* initial state is non-signaled */
		NULL);         /* name */
	/* Check if succeeded and handle errors */

	
	/* If last_error is ERROR_SUCCESS, then it means that the event was created.
	   If last_error is ERROR_ALREADY_EXISTS, then it means that the event already exists */

	return event_handle;
}

DWORD WINAPI user_interface(int inputmode) // inputmode=0 for human input. inputmode=1  for file input.
{
	BOOL is_success;
	char* temp=NULL;
	HANDLE SendStr_event=NULL;
	char recieved_string[256];
	
	if (inputmode == FILE_MODE)// FILE_MODE
	{
		int username_flag = 1;
	
		FILE* fp = fopen(global_inputfile, "r");
		if (fp == NULL) {
			perror("Error opening file");
			return(-1);
		}
		while(fgets(recieved_string, 200, fp) != NULL) {// main loop 
			if (recieved_string[strlen(recieved_string) - 1] == '\n')//removing the end of line character.
				recieved_string[strlen(recieved_string) - 1] = '\0';
			if (username_flag == 1) {//recieving the username
				strcpy(username, recieved_string);
				if (STRINGS_ARE_EQUAL(recieved_string, "exit"))// exiting program
					return 0;
				temp = encryption(&recieved_string, NEW_USER_REQUEST);
				strcpy(SendStr_global, temp);
				is_success = SetEvent(Send_event);
				username_flag = 0;
			}
			else // play/message 
			{

				if (STRINGS_ARE_EQUAL(recieved_string, "exit"))// exiting program
					return 0;
				char* temp2 = (char*)malloc(sizeof(recieved_string));
				strcpy(temp2, &recieved_string);
				char * mode;
				mode = strtok(temp2, " ");

				if (STRINGS_ARE_EQUAL(mode, "play")) {

					temp = encryption(&recieved_string, PLAY_REQUEST);
					WaitForSingleObject(turn_event, INFINITE);
					strcpy(SendStr_global, temp);
					ResetEvent(turn_event);
					is_success = SetEvent(Send_event);
				}
				
				else if (STRINGS_ARE_EQUAL(mode, "message")) {
					temp = encryption(&recieved_string, SEND_MESSAGE);
					strcpy(SendStr_global, temp);
					is_success = SetEvent(Send_event);

				}
				
				
			}

			puts(recieved_string);
		}
		fclose(fp);
		free(temp);
		
		return(0);
	}
	
	else if (inputmode == HUMAN_MODE)//HUMAN_MODE
	{
		
		DWORD last_error;
		printf("Insert username:");
		gets_s(recieved_string, sizeof(recieved_string)); //Reading a string from the keyboard
		strcpy(username, recieved_string);
		if (STRINGS_ARE_EQUAL(recieved_string, "exit"))// exiting program
		{
			return 0;

		}
		temp = encryption(&recieved_string, NEW_USER_REQUEST);
		if (Send_event == NULL)
		{
			printf("cannot create event");
			return -1;
		}
		strcpy(SendStr_global, temp);
		is_success = SetEvent(Send_event);
		// waiting for the game to start 

		while (1)
		{
			
			gets_s(recieved_string, sizeof(recieved_string));//Reading a string from the keyboard
			if (recieved_string == "")
				continue;
			if (STRINGS_ARE_EQUAL(recieved_string, "exit"))// exiting program
			{
				return 0;

			}
			char* temp2 = (char*)malloc(sizeof(recieved_string));
			strcpy(temp2, &recieved_string);
			char * pch;
			pch = strtok(temp2, " ");
			if (STRINGS_ARE_EQUAL(pch, "play")) {
				temp = encryption(&recieved_string, PLAY_REQUEST);
			}
			else if (STRINGS_ARE_EQUAL(pch, "message"))
				temp = encryption(&recieved_string, SEND_MESSAGE);
			else {
				printf("Error:illegal command\n");
				continue;
			}
			strcpy(SendStr_global, temp);
			is_success = SetEvent(Send_event);

			

		}




		return 0;
	}

}
/*this function executes the command that the server sent*/
int exec_protocol(message* msg, SOCKET sender) {// we will need another argument: SOCKET other_player
	int exit = 0;
	char* temp = NULL;

	
	switch (msg->message_type) {
	case NEW_USER_ACCEPTED:
		printf("Request to join accepted. number of players connected: %d.\n", atoi(msg->message_arguments));
		
		break;
	case NEW_USER_DECLINED:
		printf("Request to join was refused\n");
		// close program now
		exit = 1;
		break;
	case GAME_STARTED:
		printf("Game is on!\n");

		break;
	case BOARD_VIEW:// 42 arguments
		update_board(msg->message_arguments);
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		PrintBoard(board, hConsole);
		break;
	case TURN_SWITCH:
		printf("%s's turn\n", msg->message_arguments);
		if(STRINGS_ARE_EQUAL(username, msg->message_arguments))
			SetEvent(turn_event);// my turn
		break;
	case PLAY_ACCEPTED:
		printf("Well played\n");

		break;
	case PLAY_DECLINED:
		printf("Error:%s\n", msg->message_arguments);


		break;
	case GAME_ENDED:
		if (STRINGS_ARE_EQUAL(msg->message_arguments, "Tie"))
			printf("Game ended. Everybody wins!\n");
		else
			printf("Game ended. The winner is %s!\n", msg->message_arguments);
		exit = 1;
		break;
	case RECEIVE_MESSAGE:

		temp = (char*)malloc(sizeof*(msg->message_arguments));
		strcpy(temp, (msg->message_arguments));
		strchr(temp, ';')[0] = ':';
		remove_all_chars(temp, ';');
		printf("%s\n", temp);
		break;


	default:
		printf("Undifiend message type recieved\n");
		break;
	}
	return exit;
}

void remove_all_chars(char* str, char c) {
	char *pr = str, *pw = str;
	while (*pr) {
		*pw = *pr++;
		pw += (*pw != c);
	}
	*pw = '\0';
}
/*change the recieved message to the accepted format.*/
char* encryption(char* user_input, int mode)
{

	char* temp = NULL;
	char* temp2;
	switch (mode) {
	case(NEW_USER_REQUEST):
		temp = (char*)malloc(sizeof(user_input) + sizeof(server_Messages[mode]) + 10 * sizeof(char));
		strcpy(temp, server_Messages[mode]);
		strcat(temp, ":");
		strcat(temp, user_input);
		break;
	case(PLAY_REQUEST):
		temp = (char*)malloc(sizeof(user_input) + sizeof(server_Messages[mode]) + 10 * sizeof(char));
		strcpy(temp, server_Messages[mode]);

		temp2 = strchr(user_input, ' ');
		temp2[0] = ':';
		strcat(temp, temp2);
		
		break;
	case(SEND_MESSAGE):
		temp = (char*)malloc(sizeof(user_input) + sizeof(server_Messages[mode]) + 100 * sizeof(char));
		strcpy(temp, server_Messages[mode]);
		temp2 = strchr(user_input, ' ');
		temp2[0] = ':';
		int counter = 0;
		while (temp2[counter] != '\0')// put ";_;" insted of '_'
		{
			if (temp2[counter] == ' ') {
				insert_substring(temp2, "; ;", counter + 1);
				counter += 2;
			}
			else
				counter++;
		}
		strcat(temp, temp2);

		

		break;

	default:
		printf("Error, undefiened mode (cannot create message).\n");
		break;
	}

	return temp;
}

void insert_substring(char *a, char *b, int position)
{
	char *f, *e;
	int length;

	length = strlen(a);

	f = substring(a, 1, position - 1);
	e = substring(a, position, length - position + 1);

	strcpy(a, "");
	strcat(a, f);
	free(f);
	strcat(a, b);
	length = strlen(a);

	e++;
	strcat(a, e);
	e--;
	free(e);
}
char *substring(char *string, int position, int length)
{
	char *pointer;
	int c;

	pointer = malloc(length + 1);


	for (c = 0; c < length; c++)
		*(pointer + c) = *((string + position - 1) + c);

	*(pointer + c) = '\0';

	return pointer;
}


message* process_message(const char* message_text) {
	char message_type_string[MAX_MESSAGE_TYPE_SIZE];
	int message_type_size;
	char *token;
	int message_type_code;
	char * lp_params = NULL;
	message * proccessed_message;
	proccessed_message = (message*)malloc(sizeof(message));

	/* get the message type */
	if ((lp_params = strchr(message_text, ':')) == NULL)//parameterless message type
		message_type_code = get_message_code(message_text);

	else {
		message_type_size = (int)(lp_params - message_text);//lp_params-message=length of the first part of the message, e.g.:SEND_MESSAGE ==> 11 chars
		memcpy(message_type_string, message_text, message_type_size);//extract the first time of the message
		message_type_string[message_type_size] = '\0'; //add null terminator
		message_type_code = get_message_code(message_type_string);
	}

	if (message_type_code >= 0)
	{
		char *temp = (lp_params)+1;

		proccessed_message->message_arguments = temp;
	}
	else
	{
		proccessed_message->message_arguments = NULL;
	}

	proccessed_message->message_type = message_type_code;

	return proccessed_message;


}

void PrintBoard(int board[][BOARD_WIDTH], HANDLE consoleHandle)
{

	int row, column;
	//Draw the board
	for (row = 0; row < BOARD_HEIGHT; row++)
	{
		for (column = 0; column < BOARD_WIDTH; column++)
		{
			printf("| ");
			if (board[row][column] == RED_PLAYER)
				SetConsoleTextAttribute(consoleHandle, RED);

			else if (board[row][column] == YELLOW_PLAYER)
				SetConsoleTextAttribute(consoleHandle, YELLOW);

			printf("O");

			SetConsoleTextAttribute(consoleHandle, BLACK);
			printf(" ");
		}
		printf("\n");

		//Draw dividing line between the rows
		for (column = 0; column < BOARD_WIDTH; column++)
		{
			printf("----");
		}
		printf("\n");
	}


}

void update_board(char* str)
{
	int i, j;

	remove_all_chars(str, ';');
	if (strlen(str) != BOARD_HEIGHT * BOARD_WIDTH)
	{
		printf("wrong number of arguments:%d\n", strlen(str));
	}

	for (i = 0; i < BOARD_HEIGHT; i++)
		for (j = 0; j < BOARD_WIDTH; j++)
		{
			switch (str[0])
			{
			case '0':
				board[i][j] = 0;
				break;
			case '1':
				board[i][j] = RED_PLAYER;
				break;
			case '2':
				board[i][j] = YELLOW_PLAYER;
				break;


			}
			str++;
		}


}

int get_message_code(const char* message_type) {
	int i;

	for (i = 0; i < 9; i++) {
		
		if (STRINGS_ARE_EQUAL(message_type, client_Messages[i]))
			return i;

	}
	return -1;
}
/*this function create the threads:send thread, recieve thread and user interface thread.*/
void mainClient(mode,inputfile,logfile,port)// mode can be HUMAN_MODE or FILE_MODE
{ 
	Send_event =create_event_simple();
	turn_event= create_event_simple();
	int inputmode = mode;
	global_inputfile = inputfile;
	global_logfile = logfile;
	logfile_pointer = fopen(logfile,"w+");
	
	if (logfile_pointer == NULL)
	{
		printf("cannot open logfile\n");
		return 0;
	}
	logfile_mutex = CreateMutex(NULL, FALSE, NULL);
	
	SOCKADDR_IN clientService;
	HANDLE hThread[3];
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR);
	clientService.sin_port = htons(port);

	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("Failed connecting to server on %s:%d. Exiting\n",SERVER_ADDRESS_STR,port);
		WSACleanup();
		return;
	}
	printf("Connected to server on %s:%d. \n",SERVER_ADDRESS_STR,port);
	 
	hThread[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)SendDataThread,
		NULL,
		0,
		NULL
	);
	
	if (hThread[0] == NULL)
	{
		printf("cannot open send Thread");
		WSACleanup();
		return;
	}

	hThread[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvDataThread,
		NULL,
		0,
		NULL
	);

	if (hThread[1] == NULL)
	{
		printf("cannot open recv Thread");
		WSACleanup();
		return;
	}
	hThread[2] = CreateThread(
		NULL,
		0,
		user_interface,
		inputmode,
		0,
		NULL
	);

	if (hThread[2] == NULL)
	{
		printf("cannot open user interface Thread.\n");
		WSACleanup();
		return;
	}

	WaitForMultipleObjects(3, hThread, FALSE, INFINITE);
	TerminateThread(hThread[0], 0x555);
	TerminateThread(hThread[1], 0x555);
	fclose(logfile_pointer);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	CloseHandle(hThread[2]);
	CloseHandle(logfile_mutex);
	CloseHandle(turn_event);
	CloseHandle(Send_event);
	CloseHandle(hConsole);
	closesocket(m_socket);
	WSACleanup();

	return;


}




