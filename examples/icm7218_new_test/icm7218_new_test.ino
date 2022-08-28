// Example sketch for ICM7218 library
// https://github.com/Andy4495/ICM7218
//
// This sketch uses the new, simplified methods
// added in version 2.0.0 of the library
//
// 1.0.0    08/25/2022 Andy4495 Initial version

#include "ICM7218.h"

// Constructor: Confirm/update these pin numbers match your Arduino->ICM7x28 connection
// myLED(ID0, ID1, ID2, ID3, ID4, ID5, ID6, ID7, mode, write)
ICM7218 myLED(A2, 3, 4, 5, 6, 7, 8, 9, A0, A1); // Update per your configuration

#define TEST_DELAY 3000UL
unsigned long prevMillis;
uint8_t test_sequence = 0;

#define DISPLAY_SIZE 8
char buffer[DISPLAY_SIZE];
char buffer2[17];

void setup() {

  Serial.begin(9600);
  Serial.println("ICM7218 Library New Example Sketch");

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
        Serial.println("CODEB decodee: 45670123");
        myLED = "45670123";
        myLED.print();
        prevMillis = millis();
        break;

      case 5:
        myLED.setMode(ICM7218::CODEB);
        Serial.println("CODEB decodee: HELP-89");
        myLED = "HELP-89";
        myLED.print();
        prevMillis = millis();
        break;

      case 6:
        Serial.println("CODEB with decimal points after 3nd and 7th character");
        myLED.dots = 0x20 | 0x02; 
        myLED.print();
        myLED.dots = 0; 
        prevMillis = millis();
        break;
 
 // DIRECT mode examples
      case 7:
        myLED.setMode(ICM7218::DIRECT);
        Serial.println("DIRECT mode examples.");
        Serial.println("Display: abcdefgh");
        myLED[0] = 'a';
        myLED[1] = 'b';
        myLED[2] = 'c';
        myLED[3] = 'd';
        myLED[4] = 'e';
        myLED[5] = 'f';
        myLED[6] = 'g';
        myLED[7] = 'h';

        myLED.convertToSegments();
        myLED.print();
        prevMillis = millis();
        break;

      case 8:
        Serial.println("Numerals: 01234567 using external buffer");
        for (int i = 0; i < DISPLAY_SIZE; i++ ) {
          buffer[i] = i + '0';
        }
        myLED.convertToSegments(buffer);
        myLED.print(buffer);
        prevMillis = millis();
        break;

      case 9:
        Serial.println("Numerals: 76543210 using internal buffer");
        for (int i = 0; i < DISPLAY_SIZE; i++ ) {
          myLED[DISPLAY_SIZE - i - 1] = i + '0';
        }
        myLED.convertToSegments();
        myLED.print();
        prevMillis = millis();
        break;

      case 10:
        Serial.println("Using operator= 'Hello456'");
        myLED = "Hello456";
        myLED.convertToSegments();
        myLED.print();
        prevMillis = millis();
        break;

      case 11:
        Serial.println("Single char update -- index 1 and 5 changed to 'c' and 'p'. Not supported on Intersil 7218A/B.");
        myLED.print(myLED.convertToSegments('c'), 1);
        myLED.print(myLED.convertToSegments('p'), 5);
        Serial.println("");
        prevMillis = millis();
        break;

      case 12:
        Serial.println("Combine character map and individual segment control: AB.<top><middle><bottom>321.");
        myLED = "AB   321";  // First, get the characters into correct postions
        myLED.convertToSegments();
        myLED[1] = myLED[1] & 0x7f;  // Clear bit 7 to turn on decimal point
        myLED[2] = 0xc0; // Top segment (a) is ID6 = 0x40, turn off decimal point 0x80
        myLED[3] = 0x84; // Middle segment (g) is ID2 = 0x04, turn off decimal point 0x80
        myLED[4] = 0x81; // Bottom segment (d) is ID0 = 0x01, turn off decimal point 0x80
        myLED[7] = myLED[7] & 0x7f;  // Clear bit 7 to turn on decimal point
        myLED.print();
        prevMillis = millis();
        break;

      case 13:
        Serial.println("Another way to use decimal points: ab1.23.lop");
        strcpy(buffer2, "ab1.23.lop");
        myLED.convertToSegments(buffer2);
        myLED = buffer2;
        myLED.print();
        prevMillis = millis();
        break;

      default:
        // Cycle around to the first test again (don't update prevMillis)
        test_sequence = 0;
        Serial.println("No test, cycling back to first test.");
        break;
    }
  }
}