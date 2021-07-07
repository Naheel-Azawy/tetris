#include <KeyChainino.h>

// timer counter
unsigned long timer1_count_;

// game main variable
game tetris;

// unset when sleeping
bool game_started = true;

// used for buttons long press in check_btns()
bool prev_clicked = false;

ISR(TIMER1_OVF_vect) {
  // disable Timer1 interrupts
  TIMSK1 &= ~(1 << TOIE1);
  TCNT1 = timer1_count_;  // load counter for next interrupt
  TIMSK1 |= (1 << TOIE1); // enable timer overflow interrupt
}

ISR(INT1_vect) { // BUTTON_A INTERRUPT
  // do nothing
}

ISR(INT0_vect) { // BUTTON_B INTERRUPT
  // do nothing
}

int rand_int() {
  return random(0, 100);
}

void rand_seed() {
  randomSeed(analogRead(A0));
}

void check_btns() {
  if (!digitalRead(BUTTON_A) || !digitalRead(BUTTON_B)) {
    // any button is pressed

    if (prev_clicked) {
      // previously clicked once
      delay(2);
    } else {
      // just clicked
      delay(80);
    }

    if (!digitalRead(BUTTON_A) && !digitalRead(BUTTON_B)) {
      // both buttons clicked
      tetris.msg = ROTATE1;
      delay(80);
    } else {
      if (!digitalRead(BUTTON_A)) {
        // A is clicked
        tetris.msg = LEFT;
      } else if (!digitalRead(BUTTON_B)) {
        // B is clicked
        tetris.msg = RIGHT;
      }
      prev_clicked = true;
    }

  } else {
    // nothing is clicked
    prev_clicked = false;
  }
}

void draw(game *g) {
  for (int j = T; j < g->h + T; ++j) {
    check_btns();
    for (int i = 0; i < g->w; ++i) {
      KC.pixel(i, j - T, CELL(g->board_dr, i, j));
    }
  }
  KC.display();
}

void draw_game_over(game *g) {
  KC.clear();
  KC.display();
  static char buf[24];
  sprintf(buf, "GAME OVER %d", g->score);
  KC.scrollText(buf, 1);
  delay(4000);
  KC.clear();
  KC.display();
  game_started = false;
}

void setup() {
  // initialize and clear display
  KC.init();
  KC.clear();
  KC.display();

  // configure buttons pins
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  // initialize Timer 1
  timer1_count_ = 64900;
  noInterrupts();          // disable interrupts
  TCCR1A = 0;              // normal operation
  TCCR1B = 0;              // clear prescaler
  TCNT1 = timer1_count_;   // load counter
  TCCR1B |= (1 << CS10);   // prescaler = 1024
  TIMSK1 |= (1 << TOIE1);  // enable timer overflow
  interrupts();            // enable all interrupts

  // initialize the game
  tetris.w = 12;
  tetris.h = 12;
  game_init(&tetris);
  game_started = true;

  KC.goSleep();
}

void loop() {
  if (game_started) {
    check_btns();
    game_loop(&tetris);
  } else {
    KC.goSleep(); // sleep to reduce power
    game_init(&tetris);
    game_started = true;
  }
}
