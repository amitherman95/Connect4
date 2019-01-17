

/*
*	Name:messages.c
*	messages module implementation
*/

#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include "messages.h"
#include <stdio.h>
#include "socket.h"
#include <stdlib.h>
#include "game.h"

#define MAX_MESSAGE_TYPE_SIZE 19 //according to to what we are given, that is 18 characters + null terminator


/* Free memory	*/
void delete_message(message* lp_msg) {

	free(lp_msg);
}
