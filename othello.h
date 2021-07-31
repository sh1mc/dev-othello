#ifndef OTHELLO_H
#include <linux/kernel.h>
#include <linux/slab.h>
#include "libothello.h"

#define BOARD_SIZE 8
#define CANVAS_HEIGHT (BOARD_SIZE * 2 + 1)
#define CANVAS_WIDTH (BOARD_SIZE * 4 + 2) // contains '\n'

typedef struct {
  State board[BOARD_SIZE][BOARD_SIZE];
} Board;

typedef struct {
  Board *state;
  bool turn;
} Othello;

typedef struct {
  char canvas[CANVAS_HEIGHT][CANVAS_WIDTH];
} Canvas;

void *xmalloc(size_t size, gfp_t gfptype);

Canvas *new_Canvas(void);
void free_Canvas(Canvas *canvas);
void clear_Canvas(Canvas *canvas);

Board *new_Board(void);
void free_Board(Board *board);
Board *dup_Board(Board *src_board);

Othello *new_Othello(void);
void free_Othello(Othello *othello);
void set_state_Othello(Othello *othello, size_t x, size_t y, State value);
State get_state_Othello(Othello *othello, size_t x, size_t y);

void draw(void);
int reverse_num(size_t x, size_t y, bool turn);
void reverse(Board *board, size_t x, size_t y, bool turn);

void board_write(int offset);
bool writable(size_t x, size_t y, bool turn);

long get_current_turn(void);
long set_current_turn(bool turn);
long set_board_state(int x, int y, State state);
long reset_game(void);

extern Othello *othello;
extern Canvas *canvas;

#endif
