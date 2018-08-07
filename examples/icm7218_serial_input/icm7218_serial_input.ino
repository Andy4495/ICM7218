// Example program for ICM7218 library
// https://gitlab.com/Andy4495/ICM7218
//
// This sketch displays strings received over serial port
//
// 1.0.0    03/09/2018  A.T.   Original
// 1.2.0    08/07/2018  A.T.   Raname display() and wakeup() methods
//
// First two bytes received should be a "mode digit" followed by a colon
// "mode digit" can be one of:
//   '0', 'H', 'h' - Print using HEXA mode
//   '2', 'C', 'c' - Print using CODEB mode
//   '3', 'D', 'd' - Print using DIRECT mode
//   '4', 'S', 's' - Put LED in SHUTDOWN (low power) mode (remaining characters are ignored)
//   '5', 'W', 'w' - Put LED in WAKEUP (powered on) mode (remaining characters are ignored)
//   '6',          - Cylon mode (Ground Pin 10 to exit)
//
// Any other starting characters will be assumed to mean CODEB mode
// Note that the first two characters will always be interpreted as mode control characters,
// and therefore will not be printed.
// Only the 3rd through 10th characters will be printed
// That is, character array positions 2 through 9 are printed.
//
// For example:
// - If the received character string is "1:7777ffff", then 7777ffff will be displayed on LED (HEXA mode)
// - If the received character string is "2:4444-help", then 4444-help will be displayed on LED (CODEB mode)
// - If the received character string is "12345678", then 345678 (left-aligned) will be displayed on LED (default to CODEB mode),
//   since the first two digits are always interpreted as control characters, and since they are not standard format (missing colon),
//   then interpret the rest of the string as CODEB

#include <ICM7218.h>

// Configure the 10 OUTPUT pins used to interface with ICM7218: D0-D7, mode, write
ICM7218 myLED(16, 3, 4, 5, 6, 7, 8, 9, 14, 15);

#define BUTTON1 10
#define BUTTON2 11

// how much serial data we expect before a newline
const unsigned int MAX_INPUT = 50;
char input_line [MAX_INPUT];

// For Cylon mode
char convertBuffer[16];
int i;


void setup() {
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  Serial.begin(9600);

  Serial.println("Initiating CYLON mode.");
  Serial.println("Ground pin 10 to continue to serial input mode.");
  Serial.println(" ");

  myLED.setMode(ICM7218::DIRECT);
  memset(convertBuffer, 0 | ICM7218::DP, 15);  // Clear the buffer (empty character in DIRECT mode)
  convertBuffer[15] = 0;                       // Null terminator

  // Put the cylon symbol in the middle of the buffer
  convertBuffer[7] = myLED.convertToSegments('!');

  while (digitalRead(BUTTON1) == HIGH) {
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

  Serial.println("Ready for serial input string: ");
  Serial.println("  For HEXA, start with '1:'");
  Serial.println("  For CODEB, start with '2:'");
  Serial.println("  For DIRECT, start with '3:'");
  Serial.println("  For SHUTDOWN mode, start with '4:'");
  Serial.println("  For WAKEUP mode, start with '5:'");
  Serial.println("  For CYLON mode, start with '6:'");
}

void loop() {
  // if serial data available, process it
  while (Serial.available () > 0)
    processIncomingByte (Serial.read ());
}

// here to process incoming serial data after a terminator received
void process_data (char * data, int size)
{
  int power_command = 0;    // Flag if the command was for a power mode
  ICM7218::CHAR_MODE mode = ICM7218::CODEB;     // Default to CODEB

  switch (data[0]) {
    case '1':
    case 'H':
    case 'h':
      if (data[1] == ':') mode = ICM7218::HEXA;
      break;

    case '2':
    case 'C':
    case 'c':
      if (data[1] == ':') mode = ICM7218::CODEB;
      break;

    case '3':
    case 'D':
    case 'd':
      if (data[1] == ':') mode = ICM7218::DIRECT;
      // In DIRECT mode, need to pad with empty segments since there isn't a null terminator
      for (i = size + 2; i < 10; i++)
        data[i] = 0 | ICM7218::DP;
      myLED.convertToSegments(data + 2);
      break;

    case '4':
    case 'S':
    case 's':
      if (data[1] == ':') {
        power_command = 4;
        myLED.displayShutdown();
      }
      break;

    case '5':
    case 'W':
    case 'w':
      if (data[1] == ':') {
        power_command = 5;
        myLED.displayWakeup();
      }
      break;

    case '6':
      if (data[1] == ':') {
        Serial.println("Entering Cylon mode. Ground pin 10 to exit.");
        myLED.setMode(ICM7218::DIRECT);

        while (digitalRead(BUTTON1) == HIGH) {
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
        Serial.println("Exiting Cylon mode. ");
      }
      break;

    default:
      break;
  }

  if (strncmp(data, "6:", 2) != 0) {         // Only print to LED if not Cylon mode
    if (power_command == 0) {
      myLED.setMode(mode);
      myLED.print(data + 2);

      switch (mode) {
        case ICM7218::HEXA:
          Serial.print("HEXA: ");
          Serial.println(data + 2);
          break;

        case ICM7218::CODEB:
          Serial.print("CODEB: ");
          Serial.println(data + 2);
          break;

        case ICM7218::DIRECT:
          Serial.println("DIRECT");
          break;

        default:
          Serial.println("Invalid mode.");
          break;
      }
    }
    else {
      if (power_command == 4) Serial.println("Shutdown mode.");
      else Serial.println("Wakeup mode.");
    }
  }
}

void processIncomingByte (const byte inByte)
{
  static unsigned int input_pos = 0;
  int input_size;

  switch (inByte)
  {
    case '\n':   // end of text
      input_line[input_pos] = 0;  // terminating null byte
      input_size = input_pos + 1;

      // terminator reached! process input_line here ...
      input_line[13] = '\0';
      process_data(input_line, input_size);

      // reset buffer for next time
      input_pos = 0;
      break;

    case '\r':   // discard carriage return
      break;

    default:
      // keep adding if not full ... allow for terminating null byte
      if (input_pos < (MAX_INPUT - 1)) {
        input_line [input_pos++] = inByte;
      }
      break;
  }  // end of switch
} // end of processIncomingByte
