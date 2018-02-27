// Test program ICM7218 library
// 
// 1.0.0    02/27/2018  A.T.   Original


#include <ICM7218.h>

// Configure the 10 OUTPUT pins used to interface with ICM7218: D0-D7, mode, write
ICM7218 myLED(16, 3, 4, 5, 6, 7, 8, 9, 14, 15);

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
  myLED.shutdown();
  Serial.println("Shutdown 5 seconds");
  delay(5000);
  // Wakeup turns the display back on
  myLED.wakeup();
  Serial.println("Wake up 2 seconds, then loop again.");
  delay(2000);

}



