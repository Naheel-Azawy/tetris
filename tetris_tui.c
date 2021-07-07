#include "tetris.h"

#include <stdio.h>    // for printing
#include <stdlib.h>   // rand
#include <time.h>     // for srand
#include <sys/time.h> // for millis
#include <errno.h>    // for delay
#include <pthread.h>  // for non-blocking input
#include <termios.h>  // termios, TCSANOW, ECHO, ICANON
#include <unistd.h>   // STDIN_FILENO

typedef struct {
  bool  ascii;
  bool  mouse;
  char *empty;
  char *block;
  char *vline;
  char *hline;
  char *corner_nw;
  char *corner_ne;
  char *corner_sw;
  char *corner_se;
} tui;

ul millis() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void delay(ul msec) {
  // https://stackoverflow.com/a/1157217/3825872
  struct timespec ts;
  int res;
  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;
  do {
    res = nanosleep(&ts, &ts);
  } while (res && errno == EINTR);
  return;
}

int rand_int() {
  return rand();
}

void rand_seed() {
  srand(time(NULL));
}

void put(char *str) {
  fputs(str, stdout);
}

void draw_init(game *g) {
  tui *ui = (tui *) g->helper;
  if (ui->ascii) {
    ui->empty = "  ";
    ui->block = "##";
    ui->vline = "|";
    ui->hline = "--";
    ui->corner_nw = "+";
    ui->corner_ne = "+";
    ui->corner_sw = "+";
    ui->corner_se = "+";
  } else {
    ui->empty = "  ";
    ui->block = "\e[8;37;47m  \e[0m";
    ui->vline = "│";
    ui->hline = "──";
    ui->corner_nw = "╭";
    ui->corner_ne = "╮";
    ui->corner_sw = "╰";
    ui->corner_se = "╯";
  }
  put("\033[2J"); // clear screen
  if (ui->mouse) {
    put("\033[?1000;1006;1015h"); // enable mouse
  }
}

void draw(game *g) {
  tui *ui = (tui *) g->helper;
  put("\033[0;0H");
  for (int j = T; j < g->h + T; ++j) {
    put(ui->vline);
    for (int i = 0; i < g->w; ++i) {
      if (CELL(g->board_dr, i, j) != 0) {
        put(ui->block);
      } else if (j == T - 1) {
        put(" .");
      } else {
        put(ui->empty);
      }
    }
    put(ui->vline);

    switch (j - T) {
    case 0:
      put(ui->corner_nw);
      for (int _ = 0; _ < T; ++_) {
        put(ui->hline);
      }
      put(ui->corner_ne);
      break;
    case 1:
    case 2:
    case 3:
    case 4:
      put(ui->vline);
      u16 *next = g->next + 1;
      for (int i = 0; i < T; ++i) {
        if (CELL(next, i, j - 1 - T) == 0) {
          put(ui->empty);
        } else {
          put(ui->block);
        }
      }
      put(ui->vline);
      break;
    case 5:
      put(ui->corner_sw);
      for (int _ = 0; _ < T; ++_) {
        put(ui->hline);
      }
      put(ui->corner_se);
      break;
    case 7:
      printf(" Shapes: %d", g->shapes);
      break;
    case 8:
      printf(" Rows:   %d", g->rows);
      break;
    case 9:
      printf(" Score:  %d", g->score);
      break;
    }

    put("\n\r");
  }

  put(ui->corner_sw);
  for (int _ = 0; _ < g->w; ++_) {
    put(ui->hline);
  }
  put(ui->corner_se);
  put("\n\r");

  if (ui->mouse) {
    put("  <-     ->     P\n\n\r");
    put("   B     R      Q\n\r");
  }
}

void draw_game_over(game *g) {
  put("\033[2J");
  put("\033[0;0H");
  for (int j = 0; j < (g->h / 2) - 4; ++j) {
    put("\n");
  }
  printf("GAME OVER\n\n\r");
  printf("Shapes: %d\n\r", g->shapes);
  printf("Rows:   %d\n\r", g->rows);
  printf("Score:  %d\n\r", g->score);
  delay(3000);
}

#include <string.h>

void *kbd_listener(void *arg) {
  game *g = (game*) arg;
  tui *ui = (tui *) g->helper;

  // allow getchar() to work without \n or EOL
  // https://stackoverflow.com/a/1798833/3825872
  static struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  bool running = true;
  char buf[16];
  int buf_count = 0;
  char c;

  while (running) {
    put("\033[K");

    c = getchar();
    if (c == 0x1b || (c >= 0x20 && c <= 0x7e && buf[0] != 0x1b)) {
      buf_count = 0;
    }
    buf[buf_count++] = c;
    buf[buf_count] = '\0';

    switch (buf[0]) {
    case 'q':
      g->msg = QUIT;
      running = false;
      break;

    case 'p':
      g->msg = PAUSE;
      break;

    case ' ':
      g->msg = BOTTOM;
      break;

    case 0x1b:
      if (buf[1] == '[') {
        switch (buf[2]) {
        case 'A': g->msg = ROTATE1; buf_count = 0; break;
        case 'B': g->msg = DOWN;    buf_count = 0; break;
        case 'D': g->msg = LEFT;    buf_count = 0; break;
        case 'C': g->msg = RIGHT;   buf_count = 0; break;

        case '<': // mouse
          if (ui->mouse &&
              buf[3] == '0' && buf[4] == ';' &&
              buf[strlen(buf) - 1] == 'm') {

            int x, y;
            sscanf(buf + 5, "%d;%dm", &x, &y);

            if (y == g->h + 2) {
              if (x > 0 && x < 8) {
                g->msg = LEFT;
              } else if (x < 12) {
                g->msg = RIGHT;
              } else {
                g->msg = PAUSE;
              }
            } else if (y == g->h + 4) {
              if (x > 0 && x < 8) {
                g->msg = BOTTOM;
              } else if (x < 12) {
                g->msg = ROTATE1;
              } else {
                g->msg = QUIT;
                running = false;
              }
            }

            buf_count = 0;
          }
          break;
        }
      }
      break;
    }

  }

  // restore terminal behavior
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return NULL;
}

int help() {
  printf("usuage: tetris [-a] [-w WIDTH] [-h HEIGHT]\n");
  return 1;
}

int main(int argc, char **argv) {
  tui ui;
  ui.ascii = false;
  ui.mouse = false;

  game g;
  g.helper = &ui;
  g.w = 10;
  g.h = 20;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'a': ui.ascii = true;       break;
      case 'm': ui.mouse = true;       break;
      case 'w': g.w = atoi(argv[++i]); break;
      case 'h': g.h = atoi(argv[++i]); break;
      default:  return help();
      }
    } else {
      return help();
    }
  }

  draw_init(&g);
  game_init(&g);

  pthread_t t;
  pthread_create(&t, NULL, kbd_listener, &g);

  while (g.running) {
    game_loop(&g);
    delay(5);
  }

  pthread_join(t, NULL);
  if (ui.mouse) {
    put("\033[?1000;1006;1015l"); // disable mouse
  }

  return 0;
}
