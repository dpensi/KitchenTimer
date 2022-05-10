#include <arduino-timer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

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
unsigned long lastButtonPress = 0;

// Time variables
Timer<2> timer;

// Sound variables
bool play = false;

// 8x8 led matrix variables
Max72xxPanel matrix = Max72xxPanel(CS, 1, 1);
unsigned char display[8];

// numbers
const unsigned char numbers[][5] = {
  {
    B11100000,
    B10100000,
    B10100000,
    B10100000,
    B11100000  
  },
  {
    B11000000,
    B01000000,
    B01000000,
    B01000000,
    B11100000  
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
    B00100000,
    B11100000,
    B00100000,
    B11100000  
  },
  {
    B10100000,
    B10100000,
    B11100000,
    B00100000,
    B00100000  
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
    B10000000,
    B11100000,
    B10100000,
    B11100000  
  },
  {
    B11100000,
    B10100000,
    B00100000,
    B00100000,
    B00100000  
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
    B10100000,
    B11100000,
    B00100000,
    B11100000  
  }
};
void setup() {
  Serial.begin(9600);
  initRotaryEncoder();
  initSpeaker();
  initDisplay();
}

void loop() {
  currentStateCLK = digitalRead(CLK);
  timer.tick();
  if (isKnobRotated()) {
    updateCounter();
    if (play) play = false;
    if ( counter == 0) stopTimer();
  }

  if ( counter > 0 ) startTimer();

  handleAlarm();

  displayCounter();

  // Remember last CLK state
  lastStateCLK = currentStateCLK;
  // Put in a slight delay to help debounce the reading
  delay(10);
}

void initRotaryEncoder() {
  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
  currentStateCLK = digitalRead(CLK);
}

void initDisplay() {
  matrix.setIntensity(0);
}

void initSpeaker() {
  pinMode(BEEP, OUTPUT);
}

void updateCounter() {
  currentStateCLK = digitalRead(CLK);
  // If the DT state is different than the CLK state then
  // the encoder is rotating CW so increment
  if (digitalRead(DT) != currentStateCLK) {
    increaseCounter();
  } else {
    // Encoder is rotating CCW so decrement
    reduceCounter();
    if ( counter == 0 ) {
      play = true;
    }
  }
}

bool isKnobRotated() {
  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  return currentStateCLK != lastStateCLK  && currentStateCLK == 1;
}

bool isButtonPressed() {
  // Read the button state
  int btnState = digitalRead(SW);

  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 50) {
      return true;
    }

    // Remember last button press event
    lastButtonPress = millis();
  }
  return false;
}

void startTimer() {
  timer.every(60000, countdown);
  timer.every(1000, blip);
}

void stopTimer() {
  timer.cancel();
}

void blip() {
  Serial.println("blip");
}

void countdown() {
  reduceCounter();
}

void reduceCounter() {
  if (counter - 1 >= MIN_COUNTER) {
    counter --;
  }
  else {
    play = false;
  }
}

void increaseCounter() {
  if (counter + 1 <= MAX_COUNTER) {
    counter ++;
  }
}

void handleAlarm() {
  if (play) {
    digitalWrite(BEEP, HIGH);
  } else {
    digitalWrite(BEEP, LOW);
  }
}

int previousNumber = -1;

void displayCounter(){
  int firstDigit = counter / 10;
  int secondDigit = counter % 10;
  
  for(int i =0; i < 8; i++){
      display[i] = B00000000;
    }
  
  for( int i = 0; i < 5; i++){
    display[i] |= numbers[firstDigit][i];
  }
  for( int i = 0; i < 5; i++){
    display[i] |= numbers[secondDigit][i] >> 4;
  }
  if( counter != previousNumber ){
    Serial.println("##################");
    for(int i =0; i < 8; i++){
      Serial.println(display[i], BIN);
    }
    Serial.println("##################");
    previousNumber = counter;  
  }
  
  matrix.drawBitmap(0, 0, display, 8, 8, 1);
  matrix.write();
}
