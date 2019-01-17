/*
Name:game.c
Description:Game module
Authors:Amit Herman Raz Rajwan

*/


#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include "game.h"



/*Psuedo OOP style*/
/*members*/
int game_status = WAITING;
int player_turn= RED_PLAYER;
char player_one[30] = { 0 };
char player_two[30] = { 0 };
int num_players_ready = 0;

/*methods*/
void set_status(int status) {

	game_status = status;
}
int get_status() {
	return game_status;
}

void set_player_name(const char* name, int player_num) {
	switch (player_num) {
	case 0:
		strcpy(player_one, name);
		printf("New user:%s\n", player_one);
		break;
	case 1:
		strcpy(player_two, name);
		printf("New user:%s\n", player_two);
		break;
	}
}
const char* get_player_name( int player_num) {
	switch (player_num) {
	case 0:
		return player_one;
		break;
	case 1:
		return player_two;
		break;
	}
	return NULL;
}

void player_ready() {
	num_players_ready++;
}

int get_players_num_ready() {

	return num_players_ready;
}

BOOL is_name_unavail(const char * username) {
	if (strcmp(player_one, username) == 0 || strcmp(player_two, username) == 0)
		return TRUE;
	else
		return FALSE;

}

int insert_disc(int column, int color, int board[6][7]) {

	int j;

	/* invalid move, column full	*/
	if (board[0][column] != 0)
		return 0;

	for (j = 0; j < 5; j++) {
		if (board[j + 1][column] != 0)
			break;
	}

		board[j][column] = color;
		return 1;
}

void init_board(int board[6][7]) {
	int i, j;

	for (i = 0; i < 6; i++) {
		for (j = 0; j  < 7; j++) {
			board[i][j] = 0;
		}
	}
}

//return whose turn it is right now
int whose_turn()
{
	return player_turn;

}

void switch_turn()
{
	switch (player_turn) {
	case YELLOW_PLAYER:
		player_turn = RED_PLAYER;
		break;
	case RED_PLAYER:
		player_turn = YELLOW_PLAYER;
		break;
	}
}

int check_diag_right(int board[6][7]) {

	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 4; j++) {
			if (board[i][j] != 0
				&& board[i][j] == board[i + 1][j + 1]
				&& board[i + 1][j + 1] == board[i + 2][j + 2]
				&& board[i + 2][j + 2] == board[i + 3][j + 3])
				return board[i][j];
		}
	}
	return FALSE;
}
int check_diag_left(int board[6][7]) {

		int i, j;

		for (i = 0; i < 3; i++) {
			for (j = 3; j < 7; j++) {
				if (board[i][j] != 0
					&& board[i][j] == board[i + 1][j - 1]
					&& board[i + 1][j - 1] == board[i + 2][j - 2]
					&& board[i + 2][j - 2] == board[i + 3][j - 3])
					return board[i][j];
			}
		}
		return FALSE;
}
int check_columns(int board[6][7]) {

	int i, j;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 7; j++) {
			if (board[i][j] != 0 && board[i][j] == board[i + 1][j]
				&& board[i + 1][j] == board[i + 2][j] && board[i + 2][j] == board[i + 3][j])
				return board[i][j];
		}
	return 0;
}
int check_rows(int board[6][7]) {

	int i, j;

	for (i = 0; i < 6; i++)
		for (j = 0; j < 4; j++) {
			if (board[i][j] != 0 && board[i][j] == board[i][j + 1]
				&& board[i][j + 1] == board[i][j + 2] && board[i][j + 2] == board[i][j + 3])
				return board[i][j];
		}
	return 0;
}

//just checks all the possible ways and return the color of the player who won
	int check_win(int board[6][7]) {
		int c, r, dl, dr;
		c = check_columns(board);
		r = check_rows(board);
		dl = check_diag_left(board);
		dr = check_diag_right(board);
	
		if (c) return c;
		else if (r) return r;
		else if (dl) return dl;
		else if (dr) return dr;
		else return 0;
			
	}

	BOOL check_tie(int board[6][7]) {

		int i, j, result;

		result = TRUE;//Assuming true
		for (i = 0; i < 6; i++)
			for (j = 0; j < 7; j++)
				result = result && board[i][j] != 0;

		return result;
	}

	void set_turn(int player_color) {

		player_turn = player_color;
	}
	void init_names(){

		strcpy(player_one, "");
		strcpy(player_two, "");
	}

	void init_game(int board[6][7]) {
		set_status(WAITING);
		init_names();
		init_board(board);
		set_turn(RED_PLAYER);
		num_players_ready = 0;

	}