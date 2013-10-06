
#include <Servo.h> 
#include "Timer.h"                     //http://github.com/JChristensen/Timer

Servo servo;  // create servo object to control a servo

Timer t;

class KeySequence {
  static const int LENGTH = 4;
  static const int TIMEOUT = 3000;

  int sequence[LENGTH];

  int i;
  unsigned long last_click;

  public:
 
  KeySequence() {
    i = 0;
  }

  // Returns sequence as number if succesful, 0 otherwise
  int write(int key) {
    unsigned long mil = millis();
    if (mil - last_click > TIMEOUT) {
      i = 0;
    }
    last_click = mil;
    sequence[i] = key;
    ++i;
    if (i == LENGTH) {
      if (valid()) {
        i = 0;

        int result = 0;
        for (int j = 0; j < KeySequence::LENGTH / 2; ++j) {
          result *= 10;
          result += sequence[j*2];
        }
        return result;
      } else {
        return i = 0;
      }
    }
    return 0;
  }

  protected:

  boolean valid() {
    boolean result = true;
    for (int j = 0; j < LENGTH; ++j) {
      if ((j % 2) == 0) {
        result &= (sequence[j] > 0);
      } else {
        result &= (sequence[j] == -1);
      }
    }
    return result;
  }
};

KeySequence sequence;

//ADKeyboard Module
int adc_key_val[5] ={50, 200, 400, 600, 800 };
int NUM_KEYS = 5;
int adc_key_in;
int key=-1;
int oldkey=-1;

const int LED = 17;
const int SERVO_PIN = 9;
const int KEYPAD_PIN = 0;

const int START = 120;
const int END = 62;
const int MNM_PERIOD = 250;

const byte ADDR1 = '1';
const byte ADDR2 = '2';



boolean led2state = false;

void led2on() {
  TXLED1;
}

void led2off() {
  TXLED0;
}

void led2() {
  led2on();
  t.after(100, led2off);
  t.after(200, led2on);
  t.after(250, led2off);
}

void handle_keypad() {
  adc_key_in = analogRead(0);    // read the value from the sensor 
  key = get_key(adc_key_in);  // convert into key press
 
  if (key != oldkey) {  // if keypress is detected {
    t.after(10, handle_key_pressed);
  }
}

void transmit(int addr) {
  byte dest[5] = {'z', 'a', 'p', '0' + addr / 10, '0' + addr % 10};
  Mirf.setTADDR(dest);
  byte data = '\0';
  Mirf.send(&data);
  while(Mirf.isSending()){
  }
  
}

void handle_key_pressed() {
  adc_key_in = analogRead(KEYPAD_PIN);    // read the value from the sensor 
  key = get_key(adc_key_in);    // convert into key press
  if (key != oldkey) {   
    oldkey = key;
 
    int seq = sequence.write(key);

    if (seq == (ADDR1 - '0') * 10 + (ADDR2 - '0')) {
      dispense_mnm();
    }
  }
}

void mnm_open() {
  servo.write(END);
} 

void mnm_close() {
  servo.write(START);
}

void dispense_mnm() {
  mnm_open();
  t.after(MNM_PERIOD, mnm_close);
}


// Convert ADC value to key number
int get_key(unsigned int input) {
  int k;

  /*
    5
    3 1
    4 2
  */
  int translate[5] = {5, 3, 4, 2, 1};

  for (k = 0; k < NUM_KEYS; k++) {
    if (input < adc_key_val[k]) {
      return translate[k];
    }
  }
  if (k >= NUM_KEYS) k = -1;  // No valid key pressed
  return k;
}


void setup() {
  servo.attach(SERVO_PIN);
  servo.write(START);
  Serial.begin(9600); // 9600 bps
  led2();
  t.every(2000, led2);
  t.every(100, handle_keypad);
}

void loop() {
  t.update();
}

