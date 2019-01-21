/*
Name:game.h
Description:Game module header
Authors:Amit Herman Raz Rajwan

*/


#ifndef _GAME_H
#define _GAME_H

typedef int BOOL;
#define FALSE 0
#define TRUE 1
#define RED_PLAYER 1
#define YELLOW_PLAYER 2

/*	Game status	*/
#define WAITING 0
#define READY 1
#define RED_WIN 2
#define YELLOW_WIN 3
#define TIE 4


/*	Declarations	*/
void set_status(int status);
int get_status();
void set_player_name(const char* name, int player_num);
void player_ready();
int get_players_num_ready();
const char* get_player_name(int player_num);
BOOL is_name_unavail(const char * username);
void init_board(int board[6][7]);
int whose_turn();
int insert_disc(int column, int color, int board[6][7]);
void switch_turn();
int check_win(int board[6][7]);
BOOL check_tie(int board[6][7]);
void set_turn(int player_color);
void init_game(int board[6][7]);
#endif 
