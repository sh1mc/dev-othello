#ifndef LIBOTHELLO_H
#define LIBOTHELLO_H

#include <linux/kernel.h>

typedef enum {
  DARK = 0,
  LIGHT,
  NONE,
} State;

typedef enum {
  GET_CURRENT_TURN = 0,
  SET_CURRENT_TURN,
  SET_BOARD_STATE,
  RESET_GAME
} Command;

typedef struct {
  int board_x;
  int board_y;
  State state;
  bool turn;
  // true means light's turn
} CommandData;

#endif
