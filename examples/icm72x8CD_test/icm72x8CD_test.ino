// Example sketch for ICM7218 library
// https://github.com/Andy4495/ICM7218
//
// Test code for C/D variants. Does not include DIRECT mode tests.
//
// 1.3.2    05-Dec-2023 Andy4495 Initial version

#include "ICM7218.h"

// Constructor: Confirm/update these pin numbers match your Arduino->ICM7x28 connection
// Note that on ATmega328: 17=A3, 16=A2, 15=A1, 14=A0
//      myLED(ID0, ID1, ID2, ID3, ID7, DA0, DA1, DA2, mode, write, CD); // Constructor form for C/D variants of chip
ICM7218 myLED( 19,  18,  13,  12,   6,   9,  10,  11,    2,     5,  1); // Update per your configuration

#define TEST_DELAY 3000UL
unsigned long prevMillis;
uint8_t test_sequence = 0;

#define DISPLAY_SIZE 8
char buffer[DISPLAY_SIZE];
char buffer2[17];

void setup() {

  Serial.begin(9600);
  Serial.println("ICM7218 Library test for C/D variants of chip.");

  prevMillis = millis();
}

void loop() {
  if (millis() - prevMillis > TEST_DELAY) {
    test_sequence++;
    Serial.print("Test #");
    Serial.print(test_sequence);
    Serial.print(": ");

    switch (test_sequence) {

      // HEXA decoding examples
      case 1:
        myLED.setMode(ICM7218::HEXA);
        Serial.println("HEXA decode: 76543210");
        myLED = "76543210";
        myLED.print();
        prevMillis = millis();
        break;

      case 2:
        Serial.println("HEXA decode: 89ABCDEF");
        myLED = "89ABCDEF";
        myLED.print();
        prevMillis = millis();
        break;

      case 3: 
        Serial.println("HEXA with decimal points after 2nd and 5th character");
        myLED.dots = 0x40 | 0x08; 
        myLED.print();
        myLED.dots = 0; 
        prevMillis = millis();
        break;

      // CODEB decoding examples
      case 4:
        myLED.setMode(ICM7218::CODEB);
        Serial.println("CODEB decode: 45670123");
        myLED = "45670123";
        myLED.print();
        prevMillis = millis();
        break;

      case 5:
        myLED.setMode(ICM7218::CODEB);
        Serial.println("CODEB decode: HELP-89");
        myLED = "HELP-89";
        myLED.print();
        prevMillis = millis();
        break;

      case 6:
        Serial.println("CODEB with decimal points after 3rd and 7th character");
        myLED.dots = 0x20 | 0x02; 
        myLED.print();
        myLED.dots = 0; 
        prevMillis = millis();
        break;
 
 // DIRECT mode examples not supported

      default:
        // Cycle around to the first test again (don't update prevMillis)
        test_sequence = 0;
        Serial.println("No test, cycling back to first test.");
        break;
    }
  }
}