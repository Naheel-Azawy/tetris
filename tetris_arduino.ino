#include <MD_MAX72xx.h>
#include <EasyButton.h>

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES   4
#define CLK_PIN       12
#define DATA_PIN      11
#define CS_PIN        10
#define BUTTON_A      6
#define BUTTON_B      8

#define BTN_LONG_DURATION 300
#define TEXT_SCROLL_DELAY 50

game tetris;
int  dir;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
EasyButton btn_a(BUTTON_A);
EasyButton btn_b(BUTTON_B);

void scrollText(const char *p) {
  uint8_t charWidth;
  uint8_t cBuf[8]; // this should be ok for all built-in fonts
  mx.clear();
  while (*p != '\0') {
    charWidth = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
    for (uint8_t i=0; i<=charWidth; i++) { // allow space between characters
      mx.transform(MD_MAX72XX::TSL);
      if (i < charWidth) {
        mx.setColumn(0, cBuf[i]);
      }
      delay(70);
    }
  }
}

int rand_int() {
  return random(0, 100);
}

void rand_seed() {
  randomSeed(analogRead(0));
}

void draw(game *g) {
  int mx_cols = mx.getColumnCount();
  int col_count;

  // next board
  u16 *next = g->next + 1;
  col_count = 0;
  for (int j = 0; j < T; ++j) {
    check_btns();
    mx.setColumn(mx_cols - 2 - col_count++ - 1, next[j] << 3);
  }
  mx.setColumn(mx_cols - 8, 0xff);

  // game board
  col_count = 0;
  for (int j = T; j < g->h + T; ++j) {
    check_btns();
    mx.setColumn(mx_cols - 8 - col_count++ - 1, g->board_dr[j]);
  }
}

void draw_game_over(game *g) {
  mx.clear();
  scrollText("GAME OVER");
  char buf[16];
  itoa(g->score, buf, 10);
  scrollText(buf);
  delay(3000);
  mx.clear();
}

void on_btn_a() {
  tetris.msg = ROTATE2;
}

void on_btn_a_2() {
  tetris.msg = BOTTOM2;
}

void on_btn_b() {
  tetris.msg = dir;
}

void on_btn_b_2() {
  dir = dir == LEFT ? RIGHT : LEFT;
  tetris.msg = dir;
}

void check_btns() {
  btn_a.read();
  btn_b.read();
}

void setup() {
  mx.begin();
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, 1);

  btn_a.begin();
  btn_a.onPressed(on_btn_a);
  btn_a.onPressedFor(BTN_LONG_DURATION, on_btn_a_2);

  btn_b.begin();
  btn_b.onPressed(on_btn_b);
  btn_b.onPressedFor(BTN_LONG_DURATION, on_btn_b_2);

  dir = LEFT;
  tetris.w = 8;
  tetris.h = 24;

  game_init(&tetris);
}

void loop() {
  check_btns();
  game_loop(&tetris);
}
