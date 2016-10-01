#include "Keyboard.h"

// Number of columns and rows
#define COLUMNS 4
#define ROWS 5

// NUM_LOCK Led Pin
#define NUM_LOCK_LED 2

// Debounce 
#define DEBOUNCE_MS 20

// Used to keep track of the led status
int ledStatus = -1;

// Column pins
int inputPins[COLUMNS] = { 10, 16, 14, 15 } ;

// Row pins
int strobePins[ROWS] = { 9, 8, 7, 6, 5 };

unsigned long keyState[ROWS][COLUMNS] = {{0}};

int keycode[ROWS][COLUMNS] = { { KEY_NUM_LOCK, KEY_NUM_SLSH, KEY_NUM_TMES, KEY_NUM_MNUS },
                               { KEY_NUM_7,    KEY_NUM_8,    KEY_NUM_9,    KEY_NUM_PLUS },
                               { KEY_NUM_4,    KEY_NUM_5,    KEY_NUM_6,    0            },
                               { KEY_NUM_1,    KEY_NUM_2,    KEY_NUM_3,    KEY_NUM_ENTR },
                               { KEY_NUM_0,    0,            KEY_NUM_DOT,  0            } };


bool debounce(unsigned long t_now, unsigned long t_prev) {
  unsigned long diff = t_now - t_prev;

  if (diff <= DEBOUNCE_MS)  {
    return true;
  }
  return false;
}


void setup() {
  int cnt;

  pinMode(NUM_LOCK_LED, OUTPUT);
  
  // Setup row pins as output and set the output as HIGH
  // At the start of a loop, the current row is set a LOW
  for (cnt = 0; cnt < ROWS; cnt++) {
    pinMode(strobePins[cnt], OUTPUT);
    digitalWrite(strobePins[cnt], HIGH);
  }

  // Setup column pins as pullup
  for (cnt = 0; cnt < COLUMNS; cnt++) {
    pinMode(inputPins[cnt], INPUT_PULLUP);
  }

  // Start the keyboard library
  Keyboard.begin();
}


void checkNumlockLed() {
  // Read all led status
  int statusLed = Keyboard.getKeyboardLeds();

  // Check if the led status is different from the previous one
  if (ledStatus != statusLed) {
    // Check if the numlock led status and turn the led on the board
    if (statusLed & 0x01) {
      digitalWrite(NUM_LOCK_LED, HIGH);
    } else {
      digitalWrite(NUM_LOCK_LED, LOW);
    }
  // Update the last status
    ledStatus = statusLed;
  }
}

// Using this to read a row from the keyboard matrix
int strobeRow = 0;

void loop() {
  unsigned long tickNow = millis();
  // Since we use non zero to indicate pressed state, we need
  // to handle the edge case where millis() returns 0
  if (tickNow == 0) tickNow = 1;

  
  // If we reach the last row, restart to the first one
  if (strobeRow >= ROWS) strobeRow = 0;
  
  // Set the row to LOW
  digitalWrite(strobePins[strobeRow], LOW);
  delay(2); 

  // Go through all columns to read a input
  for (int cnt = 0; cnt < COLUMNS; cnt++) {
    // Ignore state change for pin if in debounce period
    if (keyState[strobeRow][cnt] != 0)
      if (debounce(tickNow, keyState[strobeRow][cnt])) {
        continue;
      }

    if (digitalRead(inputPins[cnt])) {
      if (keyState[strobeRow][cnt] != 0) {
        Keyboard.release(keycode[strobeRow][cnt]);
        keyState[strobeRow][cnt] = 0;
      }
    } else {
      if (keyState[strobeRow][cnt] == 0) {
        Keyboard.press(keycode[strobeRow][cnt]);
        keyState[strobeRow][cnt] = tickNow;
      }
    }
  }

  // Check the numlock led
  checkNumlockLed();

  // Set the row back to HIGH
  digitalWrite(strobePins[strobeRow], HIGH);
  // Increment the strobRow variable
  strobeRow++;
  delay(5);
}
