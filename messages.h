//Name:messages.h
//Description: headers for the message module
//Authors:Amit Herman and Raz Rajwan
#include <string.h>
#include <stdio.h>
#include "socket.h"


typedef struct message {

	int message_type;
	char * message_arguments;

}message;


/*	Declarations	*/
void delete_message(message* lp_msg);
