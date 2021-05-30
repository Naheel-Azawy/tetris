#include "tetris.h"

#include <emscripten.h>
#include <stdlib.h> // rand

game tetris;

// game_board (w * h), next_board (T * T),
// score (1), shapes (1), rows (1), game_over_flag (1), draw_flag (1)
// input_msg (1), millis_now (1)
int *data;
int data_len;

int get_data_len(int w, int h) {
  return (w * h) + (T * T) + 7;
}

ul millis() {
  return data[data_len - 1];
}

int rand_int() {
  return rand();
}

void rand_seed() {
  srand(millis());
}

void read_keyboard(game *g) {
  g->msg = data[data_len - 2];
}

void draw(game *g) {
  for (int j = T; j < g->h + T; ++j) {
    for (int i = 0; i < g->w; ++i) {
      read_keyboard(g);
      data[g->w * (j - T) + i] = CELL(g->boardtmp, i, j);
    }
  }

  for (int j = 0; j < T; ++j) {
    for (int i = 0; i < T; ++i) {
      read_keyboard(g);
      data[(g->w * g->h) + T * j + i] = CELL(g->next, i, j);
    }
  }

  data[g->w * g->h + T * T + 0] = g->score;
  data[g->w * g->h + T * T + 1] = g->shapes;
  data[g->w * g->h + T * T + 2] = g->rows;
  data[g->w * g->h + T * T + 3] = 0; // gameover
  data[g->w * g->h + T * T + 4] = 1; // draw flag
}

void draw_game_over(game *g) {
  data[g->w * g->h + T * T + 0] = g->score;
  data[g->w * g->h + T * T + 1] = g->shapes;
  data[g->w * g->h + T * T + 2] = g->rows;
  data[g->w * g->h + T * T + 3] = 1; // gameover
  data[g->w * g->h + T * T + 4] = 1; // draw flag
}

void setup(int w, int h, int *d) {
  data_len = get_data_len(w, h);
  data = d;
  tetris.w = w;
  tetris.h = h;
  game_init(&tetris);
}

void loop() {
  read_keyboard(&tetris);
  game_loop(&tetris);
}
