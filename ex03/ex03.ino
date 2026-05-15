const int ledPin = 2;
unsigned long prevMillis = 0;

const int DOT = 150;
const int DASH = 450;
const int GAP = 150;
const int LETTER = 500;
const int WORD = 2000;

int sequence[] = {
  0,2,0,2,0,    // S 三短
  3,
  1,2,1,2,1,    // O 三长
  3,
  0,2,0,2,0,    // S 三短
  4              // 长停顿
};

int step = 0;
int len = sizeof(sequence) / sizeof(sequence[0]);

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  unsigned long currMillis = millis();
  int cmd = sequence[step];
  unsigned long dur = 0;
  bool state = LOW;

  if (cmd == 0) { dur = DOT; state = HIGH; }
  if (cmd == 1) { dur = DASH; state = HIGH; }
  if (cmd == 2) { dur = GAP; state = LOW; }
  if (cmd == 3) { dur = LETTER; state = LOW; }
  if (cmd == 4) { dur = WORD; state = LOW; }

  if (currMillis - prevMillis >= dur) {
    prevMillis = currMillis;
    digitalWrite(ledPin, state);
    step++;
    if (step >= len) step = 0;
  }
}