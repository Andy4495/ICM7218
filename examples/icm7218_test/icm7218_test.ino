// Example sketch for ICM7218 library
// https://github.com/Andy4495/ICM7218
//
// 1.0.0    02/27/2018  A.T.   Original
// 1.1.0    03/01/2018  A.T.   Add ASCII to segment mapping in DIRECT mode
// 1.2.0    08/07/2018  A.T.   Raname display() and wakeup() methods


#include <ICM7218.h>

// Configure the 10 OUTPUT pins used to interface with ICM7218: D0-D7, mode, write
ICM7218 myLED(16, 3, 4, 5, 6, 7, 8, 9, 14, 15);

char convertBuffer[16];
char c;
int i,j;

void setup() {

  Serial.begin(9600);

}

void loop() {

  // HEXA character mode: supports hexadecimal digits 0-9 and A-F and decimal points
  myLED.setMode(ICM7218::HEXA);
  myLED.print("1.234ab.CD");
  Serial.println("HEXA:   1.234AB.CD");
  delay(5000);

  // CODEB character mode: supports digits 0-9, characters 'H', 'E', 'L', 'P',
  // space (' ') and hyphen ('-') plus decimal points
  myLED.setMode(ICM7218::CODEB);
  myLED.print("3.5 -HElp.");
  Serial.println("CODEB:  3.5 -HELP.");
  delay(5000);

  // DIRECT mode supports direct control of LED segments and decimal points
  // See ICM7218 datasheet for bit to segment definitions
  myLED.setMode(ICM7218::DIRECT);
  myLED.print("\xcb\xf1\x96\x2c\xc5\xe7\xca\xb1");
  Serial.println("Direct: special display");
  delay(5000);

  // Shutdown mode turns off the display and puts the chip in a low-power mode
  myLED.displayShutdown();
  Serial.println("Shutdown 2 seconds");
  delay(2000);
  // Wakeup turns the display back on
  myLED.displayWakeup();
  Serial.println("Wake up 2 seconds with same display as previous.");
  delay(2000);

  // More examples for DIRECT mode
  Serial.println("More DIRECT mode: ");
  strncpy(convertBuffer, "Ardu,no", 8);    // Use a comma for lower-case 'i' -- it looks a little better here
  myLED.convertToSegments(convertBuffer);
  myLED.print(convertBuffer);
  Serial.println("Arduino");
  delay(2000);

  strncpy(convertBuffer, "DEC.POINT", 10);
  myLED.convertToSegments(convertBuffer);
  myLED.print(convertBuffer);
  Serial.println("DEC.POINT");
  delay(2000);

  // Fill buffer with blank characters, no decimal point
  memset(convertBuffer, 0 | ICM7218::DP, 15);  // Decimal point is active low, so need to set bit 7 to 1 to turn off DP
  convertBuffer[15] = 0; // Null terminator
  c = myLED.convertToSegments('A');   // Get the DIRECT mode segment bit settings for letter 'A'
  convertBuffer[3] = c;               // Put 'AA' in the middle of the display
  convertBuffer[4] = c;
  convertBuffer[3] = convertBuffer[3] & ~ICM7218::DP;  // Turn on decimal point (active low) between the A's
  myLED.print(convertBuffer);
  Serial.println("   A.A   ");
  delay(2000);

  memset(convertBuffer, 0 | ICM7218::DP, 15);  //Clear the buffer again.
  convertBuffer[15] = 0;              // Null terminator
  // Now turn on each segment individually -- 8 characters, 8 segments
  convertBuffer[0] = 0x40 | ICM7218::DP;    // Segment a (top)
  convertBuffer[1] = 0x20 | ICM7218::DP;    // Segment b (top right)
  convertBuffer[2] = 0x10 | ICM7218::DP;    // Segment c (bottom right)
  convertBuffer[3] = 0x01 | ICM7218::DP;    // Segment d (bottom)
  convertBuffer[4] = 0x08 | ICM7218::DP;    // Segment e (bottom left)
  convertBuffer[5] = 0x02 | ICM7218::DP;    // Segment f (top left)
  convertBuffer[6] = 0x04 | ICM7218::DP;    // Segment g (middle)
  convertBuffer[7] = 0x7f & ~ICM7218::DP;   // All segments and decimal point

  myLED.print(convertBuffer);
  Serial.println("Individual segments: ");
  Serial.println("  a - top");
  Serial.println("  b - top right");
  Serial.println("  c - bottom right");
  Serial.println("  d - bottom");
  Serial.println("  e = bottom left");
  Serial.println("  f = top left");
  Serial.println("  g = middle");
  Serial.println("  All segments and decimal point");
  delay(5000);

  // Cylon display example
  memset(convertBuffer, 0 | ICM7218::DP, 15);  //Clear the buffer again.
  convertBuffer[15] = 0;                       // Null terminator

  // Put the cylon symbol in the middle of the buffer
  convertBuffer[7] = myLED.convertToSegments('!');

  Serial.println("Enter Cylon mode.....");
  for (j = 0; j < 5; j++) {
    // Display the symbol left to right by sending a different starting point to the print() method
    for (i = 7; i >= 0; i--) {
      myLED.print(convertBuffer + i);
      delay(100);
    }
    for (i = 1; i < 7; i++) {
      myLED.print(convertBuffer + i);
      delay(100);
    }
  }
}
