#include "linux/uaccess.h"
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#include <linux/modversions.h>
#define MODVERSIONS
#endif

#include "othello.h"
#include <asm/errno.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/types.h>

static ssize_t othello_read(struct file *, char *buf, size_t length,
                            loff_t *offset);
static ssize_t othello_write(struct file *, const char *buf, size_t length,
                             loff_t *offset);
static long othello_command(struct file *file, unsigned int code,
                            unsigned long data);
static int __init othello_init(void);
static void __exit othello_exit(void);

MODULE_DESCRIPTION("othello");
MODULE_LICENSE("GPL");
module_init(othello_init);
module_exit(othello_exit);

#define SUCCESS
#define DEVICE_NAME "othello"
#define BUF_LEN 80

static int Major;
static int write_buffer_num;

const struct file_operations fops = {.read = othello_read,
                                     .write = othello_write,
                                     .unlocked_ioctl = othello_command};

static struct miscdevice mdev = {
    .minor = 0, .name = DEVICE_NAME, .fops = &fops, .mode = 0666};

Othello *othello;
Canvas *canvas;

void *xmalloc(size_t size, gfp_t gfptype) {
  void *p = kmalloc(size, gfptype);
  if (p == NULL) {
    printk(KERN_ERR "Failed to allocate memory\n");
    panic("Failed to allocate memory\n");
  }
  return p;
}

#define New(type) ((type *)xmalloc(sizeof(type), GFP_KERNEL))

Canvas *new_Canvas(void) {
  Canvas *newcanvas = New(Canvas);
  size_t i, j;
  for (i = 0; i < CANVAS_HEIGHT; i++) {
    for (j = 0; j < CANVAS_WIDTH; j++) {
      newcanvas->canvas[i][j] = ' ';
    }
  }
  return newcanvas;
}

void free_Canvas(Canvas *canvas) { kfree(canvas); }

Board *new_Board(void) {
  Board *board = New(Board);
  size_t i, j;
  for (i = 0; i < BOARD_SIZE; i++) {
    for (j = 0; j < BOARD_SIZE; j++) {
      board->board[i][j] = NONE;
    }
  }
  return board;
}

Board *dup_Board(Board *src_board) {
  Board *dst_board = New(Board);
  size_t i, j;
  for (i = 0; i < BOARD_SIZE; i++) {
    for (j = 0; j < BOARD_SIZE; j++) {
      dst_board->board[i][j] = src_board->board[i][j];
    }
  }
  return dst_board;
}

void free_Board(Board *board) { kfree(board); }

Othello *new_Othello(void) {
  Othello *othello = New(Othello);
  othello->state = new_Board();
  othello->turn = 1;
  return othello;
}

void free_Othello(Othello *othello) {
  free_Board(othello->state);
  kfree(othello);
}

void clear_Canvas(Canvas *canvas) {
  int i, j;
  for (i = 0; i < CANVAS_HEIGHT; i++) {
    for (j = 0; j < CANVAS_WIDTH; j++) {
      canvas->canvas[i][j] = ' ';
    }
  }
}

void set_state_Othello(Othello *othello, size_t x, size_t y, State value) {
  othello->state->board[y][x] = value;
}
State get_state_Othello(Othello *othello, size_t x, size_t y) {
  return othello->state->board[y][x];
}

static int __init othello_init(void) {
  const size_t center = BOARD_SIZE / 2;
  Major = misc_register(&mdev);
  write_buffer_num = 0;
  if (Major < 0) {
    printk(KERN_ERR "Registering %s failed.\n", DEVICE_NAME);
    return Major;
  }
  printk(KERN_INFO "Assigned %s (%d).\n", DEVICE_NAME, Major);
  if (othello != NULL) {
    free_Othello(othello);
  }
  if (canvas != NULL) {
    free_Canvas(canvas);
  }
  othello = new_Othello();
  canvas = new_Canvas();
  set_state_Othello(othello, center, center, BLACK);
  set_state_Othello(othello, center - 1, center - 1, BLACK);
  set_state_Othello(othello, center - 1, center, WHITE);
  set_state_Othello(othello, center, center - 1, WHITE);
  draw();
  return 0;
}

static void __exit othello_exit(void) {
  if (othello != NULL) {
    free_Othello(othello);
  }
  if (canvas != NULL) {
    free_Canvas(canvas);
  }
  misc_deregister(&mdev);
  printk(KERN_INFO "Deergistered %s.\n", DEVICE_NAME);
}

void draw() {
  int i, j;
  for (i = 0; i < CANVAS_HEIGHT; i++) {
    for (j = 0; j < CANVAS_WIDTH; j++) {
      int board_x, board_y;
      if (j == CANVAS_WIDTH - 1) {
        canvas->canvas[i][j] = '\n';
        continue;
      }
      if (j % 4 == 0 && i % 2 == 0) {
        canvas->canvas[i][j] = '+';
        continue;
      }
      if (j % 4 == 0) {
        canvas->canvas[i][j] = '|';
        continue;
      }
      if (i % 2 == 0) {
        canvas->canvas[i][j] = '-';
        continue;
      }
      board_y = i / 2;
      board_x = j / 4;
      if (j % 4 == 2 && get_state_Othello(othello, board_x, board_y) != NONE) {
        canvas->canvas[i][j] =
            get_state_Othello(othello, board_x, board_y) == WHITE ? 'O' : '@';
        continue;
      }
      canvas->canvas[i][j] = ' ';
    }
  }
}

static ssize_t othello_read(struct file *file, char *buf, size_t length,
                            loff_t *offset) {
  long long i;
  int bytes_read = 0;
  if (*offset > (loff_t)(CANVAS_HEIGHT * CANVAS_WIDTH))
    return 0;
  draw();
  for (i = (long long)(*offset); i < (long long)(*offset) + length; i++) {
    if (i >= (size_t)(CANVAS_HEIGHT * CANVAS_WIDTH))
      break;
    put_user(canvas->canvas[i / CANVAS_WIDTH][i % CANVAS_WIDTH], buf++);
    bytes_read++;
  }
  *offset += bytes_read;
  return bytes_read;
}

static ssize_t othello_write(struct file *file, const char *buf, size_t length,
                             loff_t *offset) {
  int i;
  char *kbuf = xmalloc(sizeof(char) * (length + 1), GFP_KERNEL);
  if (copy_from_user(kbuf, buf, length) != 0) {
    printk(KERN_ERR "Read buffer failed.\n");
    return -EFAULT;
  }
  kbuf[length] = '\0';
  printk(KERN_INFO "kbuf: %s\n", kbuf);
  for (i = 0; i < length; i++) {
    if ((loff_t)i > length) {
      printk(KERN_ERR "Out of buffer range.\n");
      kfree(kbuf);
    }
    if (kbuf[i] == 'O' || kbuf[i] == '@') {
      State disk = (kbuf[i] == 'O' ? WHITE : BLACK);
      if ((disk == WHITE) == othello->turn) {
        board_write(write_buffer_num);
      }
    }
	if (kbuf[i] == '\0' || kbuf[i] == '\n') {
		write_buffer_num = 0;
		break;
	}
	write_buffer_num++;
  }
  kfree(kbuf);
  *offset += length;
  return length;
}

static const int DIRECTION[8][2] = {{0, 1},  {0, -1}, {1, 0},  {1, 1},
                                    {1, -1}, {-1, 0}, {-1, 1}, {-1, -1}};

typedef enum { X, Y } Value;

bool writable(size_t x, size_t y, bool turn) {
  return (bool)reverse_num(x, y, turn);
}

int reverse_num(size_t x, size_t y, bool turn) {
  Board *board;
  int num = 0;
  int i, j;
  if (get_state_Othello(othello, x, y) == !turn)
    return 0;
  board = dup_Board(othello->state);
  reverse(board, x, y, turn);
  for (i = 0; i < BOARD_SIZE; i++) {
    for (j = 0; j < BOARD_SIZE; j++) {
      if (board->board[i][j] != get_state_Othello(othello, j, i))
        num++;
    }
  }
  if (num <= 1)
    return 0;
  free_Board(board);
  return num;
}

void reverse(Board *board, size_t x, size_t y, bool turn) {
  int i;
  for (i = 0; i < 8; i++) {
    int x1 = x + DIRECTION[i][X];
    int y1 = y + DIRECTION[i][Y];
    int rev_num = 1;
    if (get_state_Othello(othello, x1, y1) == turn)
      continue;
    x1 += DIRECTION[i][X];
    y1 += DIRECTION[i][Y];
    while (x1 >= 0 && y1 >= 0 && x1 < BOARD_SIZE && y1 < BOARD_SIZE) {
      if (get_state_Othello(othello, x1, y1) == turn) {
        int j;
        for (j = 0; j <= rev_num; j++) {
          board->board[y + DIRECTION[i][Y] * j][x + DIRECTION[i][X] * j] = turn;
        }
      }
      if (get_state_Othello(othello, x1, y1) == NONE)
        break;
      rev_num++;
      x1 += DIRECTION[i][X];
      y1 += DIRECTION[i][Y];
    }
  }
}

void board_write(int offset) {
  int board_x, board_y;
  if (offset < 0 || offset >= BOARD_SIZE * BOARD_SIZE)
    return;
  board_x = offset % BOARD_SIZE;
  board_y = offset / BOARD_SIZE;
  if (writable(board_x, board_y, othello->turn)) {
    set_state_Othello(othello, board_x, board_y, othello->turn);
    reverse(othello->state, board_x, board_y, othello->turn);
    othello->turn = !(othello->turn);
  }
}

int count(State state) {
  int ret = 0;
  int i, j;
  for (i = 0; i < BOARD_SIZE; i++) {
    for (j = 0; j < BOARD_SIZE; j++) {
      if (get_state_Othello(othello, j, i) == state)
        ret++;
    }
  }
  return ret;
}

static long othello_command(struct file *file, unsigned int code,
                            unsigned long data) {
  int black_num, white_num;
  switch ((Command)code) {
  case GET_CURRENT_TURN:
    printk(KERN_INFO "%s\n", (othello->turn == BLACK ? "BLACK" : "WHITE"));
    break;
  case GET_INFO:
    black_num = count(BLACK);
    white_num = count(WHITE);
    printk(KERN_INFO "BLACK: %d\nWHITE:%d\n", black_num, white_num);
    break;
  case SET_CURRENT_TURN:
    if ((State)data == BLACK || (State)data == WHITE) {
      othello->turn = (State)data;
    }
    break;
  case RESET_GAME:
    printk(KERN_INFO "Under Construction\n");
    break;
  default:
    break;
  }
  return 0;
}
