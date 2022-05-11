#include <arduino-timer.h>
#include <LedControl.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

// Rotary Encoder Inputs
#define CLK 2
#define DT 3
#define SW 4
#define MAX_COUNTER 99
#define MIN_COUNTER 0

// 8x8 Led Matrix Outputs
#define DIN 11
#define CS  10
#define LM_CLK 13

// Speaker Output
#define BEEP 5

// Rotary encoder variables
int counter = 0;
int currentStateCLK;
int lastStateCLK;
unsigned long lastRotation = 0;
ClickEncoder *encoder;

// Time variables
Timer<2> timer;

// Sound variables
bool play = false;

// 8x8 led matrix variables
bool sleeping = false;
LedControl matrix = LedControl(DIN, LM_CLK, CS, 1);
unsigned char display[8];
const unsigned char numbers[][5] = { // numbers
  {
    B11100000,
    B10100000,
    B10100000,
    B10100000,
    B11100000
  },
  {
    B11100000,
    B01000000,
    B01000000,
    B01000000,
    B11000000
  },
  {
    B11100000,
    B10000000,
    B11100000,
    B00100000,
    B11100000
  },
  {
    B11100000,
    B00100000,
    B11100000,
    B00100000,
    B11100000
  },
  {
    B00100000,
    B00100000,
    B11100000,
    B10100000,
    B10100000
  },
  {
    B11100000,
    B00100000,
    B11100000,
    B10000000,
    B11100000
  },
  {
    B11100000,
    B10100000,
    B11100000,
    B10000000,
    B11100000
  },
  {
    B00100000,
    B00100000,
    B00100000,
    B10100000,
    B11100000
  },
  {
    B11100000,
    B10100000,
    B11100000,
    B10100000,
    B11100000
  },
  {
    B11100000,
    B00100000,
    B11100000,
    B10100000,
    B11100000
  }
};


void timerIsr() {
  encoder->service();
}

void setup() {
  Serial.begin(9600);
  initRotaryEncoder();
  initSpeaker();
  initDisplay();
}

void loop() {
  currentStateCLK = digitalRead(CLK);
  timer.tick();

  counter += encoder->getValue();
  if (counter < MIN_COUNTER) counter = MIN_COUNTER;
  if (counter > MAX_COUNTER) counter = MAX_COUNTER;

  if (isKnobRotated()) {

    lastRotation = millis();
    if (sleeping) {
      matrix.shutdown(0, false);
      sleeping = false;
    }

    if ( counter == 0 ) {
      play = false;
      stopTimer();
    }
    if (play) play = false;
  }

  if ( counter > 0 ) startTimer();

  handleAlarm();

  displayCounter();

  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  if (timer.empty() && ((millis() - lastRotation) > 30000)) {
    matrix.shutdown(0, true);      //sleep
    sleeping = true;
  }

  // Put in a slight delay to help debounce the reading
  delay(150);
}

void initRotaryEncoder() {
  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
  currentStateCLK = digitalRead(CLK);

  // init clickEncoder library
  encoder = new ClickEncoder(DT, CLK, SW);
  encoder->setAccelerationEnabled(true);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
}

void initDisplay() {
  matrix.shutdown(0, false);      //The MAX72XX is in power-saving mode on startup
  matrix.setIntensity(0, 5);      // Set the brightness to maximum value
  matrix.clearDisplay(0);         // and clear the display
}

void initSpeaker() {
  pinMode(BEEP, OUTPUT);
}

bool isKnobRotated() {
  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  return currentStateCLK != lastStateCLK  && currentStateCLK == 1;
}

void startTimer() {
  timer.every(60000, countdown);
  timer.every(1000, blip);
}

void stopTimer() {
  timer.cancel();
}

void blip() {
  matrix.setColumn(0, 7, B00000001);
}

void countdown() {
  reduceCounter();
  if (counter == 0) {
    play = true;
  }
}

void reduceCounter() {
  if (counter - 1 >= MIN_COUNTER) {
    counter --;
  }
  else {
    play = false;
  }
}

void handleAlarm() {
  if (play) {
    digitalWrite(BEEP, HIGH);
  } else {
    digitalWrite(BEEP, LOW);
  }
}

void displayCounter() {
  int firstDigit = counter / 10;
  int secondDigit = counter % 10;

  for ( int i = 0; i < 8; i++) {
    display[i] = 0;
    if (0 <= i && i < 5) {
      display[i] = numbers[firstDigit][i] | (numbers[secondDigit][i] >> 4);
    }
    matrix.setColumn(0, i, display[i]);
  }
}
