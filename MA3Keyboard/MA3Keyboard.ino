#include <Keyboard.h>
#define __ 0x00000000 // No Chord
#define C_ 0x00000001 // Ctrl
#define _A 0x00000010 // Alt
#define CA 0x00000011 // Ctrl+Alt
#define DEBOUNCE_MILLIS 50

// *****************************
// **** START CHANGING HERE ****
// *****************************

#define ROWS 5
#define COLS 13

// Define Row/column pin order
const int rowPins[ROWS] = {A3,A2,A1,A0,15};
const int colPins[COLS] = {8,7,6,5,4,3,2,9,0,1,11,16,10};

// What to press

// Ctrl / Alt Chord keys
const uint8_t keyMapChord[ROWS][COLS] = {
  {__,__,_A, C_,__, __,__,__, __,__,__,__, __},
  {__,CA,CA, C_,C_, __,__,__, __,__,__,__, C_},
  {__,__,__, C_,C_, __,_A,_A, __,__,__,__, __},
  {C_,__,__, _A,CA, __,_A,_A, __,__,__,__, __},
  {C_,CA,C_, C_,_A, __,__,__, __,__,__,__, __}
};

// Command key map
// see this for special keys: https://www.arduino.cc/reference/en/language/functions/usb/keyboard/keyboardmodifiers/
const uint8_t keyMap[ROWS][COLS] = { 
    {KEY_LEFT_SHIFT,'h','f',  'o','o',  'q',KEY_LEFT_ARROW,KEY_RIGHT_ARROW,  '7','8','9','+',KEY_BACKSPACE},
    {KEY_PAGE_UP,   's','p',  'm','c',  'f',      'c',           'g',        '4','5','6','t','f'},
    {KEY_PAGE_DOWN, 'l','b',  'd','a',  'p',      's',           'c',        '1','2','3','-',KEY_ESC},
    {     'l',      '[',']',  't','h',  'e',      'a',           't',        '0','.','i','a',KEY_DELETE},
    {     'p',      'g','g',  's','g',  'u',      's',           's',        KEY_LEFT_SHIFT,'/',KEY_RETURN,KEY_RETURN,KEY_DELETE}
};

// ****************************
// **** STOP CHANGING HERE ****
// ****************************

// helper vars
bool someKeyIsAlreadyPressed = false;
bool pressedKey[ROWS][COLS] = {false};
uint8_t shiftPressesCount = 0;

// hardware helper vars
unsigned long currentMillis = 0;
unsigned long hwKeyLastMillis[ROWS][COLS] = {0};
bool hwKeyFlags[ROWS][COLS] = {false};

// ----------------------------
void setup() {
  
  for(int i = 0; i < ROWS; i++){
    pinMode(rowPins[i], OUTPUT);
  }

  for(int i = 0; i < COLS; i++){
    pinMode(colPins[i], INPUT_PULLUP);
  }

  Keyboard.begin();
}

// ----------------------------
void loop() {

  // cache current loop millis, not as acurrate (which doesn't matter), but waaaay faster
  currentMillis = millis();

  // Keyboard keys scan/polling
  for (uint8_t row = 0; row < ROWS; row++) {
    
    // set the row pin low, this begin the scan for that row.
    pinMode(rowPins[row], OUTPUT);
    digitalWrite(rowPins[row], LOW);
      
    for (uint8_t col = 0; col < COLS; col++) {

      // scan column value
      bool keyIsPressed = !digitalRead(colPins[col]);

      // key changed state and debounce
      if (hwKeyFlags[row][col] != keyIsPressed && (currentMillis - hwKeyLastMillis[row][col]) > DEBOUNCE_MILLIS){
        hwKeyFlags[row][col] = keyIsPressed;
        hwKeyLastMillis[row][col] = currentMillis;

        // Shift key?
        if (keyMap[row][col] == KEY_LEFT_SHIFT)
        {
          // Pressing shift
          if (keyIsPressed){
            shiftPressesCount++;
            Keyboard.press(keyMap[row][col]);
          }

          // Releasing Shift
          if(!keyIsPressed){
            shiftPressesCount--;
            Keyboard.release(keyMap[row][col]);
          }

          // continue the loop
          continue;
        }

        // Pressing a command key:
        // only one key is allowed at once to prevent unwanted key presses and ghosting
        // if more than 1 shift keys are pressed, also skip as this WILL cause ghosting
        if (keyIsPressed && !someKeyIsAlreadyPressed && shiftPressesCount < 2){
          pressedKey[row][col] = true;
          someKeyIsAlreadyPressed = true;

          // press ctrl/alt chord keys if necessary
          if (keyMapChord[row][col] & C_){
            Keyboard.press(KEY_LEFT_CTRL);
          }
          if (keyMapChord[row][col] & _A){
            Keyboard.press(KEY_LEFT_ALT);
          }

          // press the command key
          Keyboard.write(keyMap[row][col]);

          // release chord keys
          if (keyMapChord[row][col] & C_){
            Keyboard.release(KEY_LEFT_CTRL);
          }
          if (keyMapChord[row][col] & _A){
            Keyboard.release(KEY_LEFT_ALT);
          }
        }

        // Releasing key, should be the same as the currently pressed one
        if(!keyIsPressed && pressedKey[row][col])
        {
          pressedKey[row][col] = false;
          someKeyIsAlreadyPressed = false;
        }
      }
    }

    // Set row pin to high impedance input. Effectively ends row pulse.
    digitalWrite(rowPins[row],HIGH);
    pinMode(rowPins[row],INPUT);
  }
}
