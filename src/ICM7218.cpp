/* Library to interface with Intersil/Renesas ICM7218A/B LED driver chip.
   Note that there are several chips in the 72xx family, including
   chips from Maxim.
   While these other chips provide similar capabilities, this library
   was specifically designed for the Intersil ISM7218A/B variants.
   This library may require modifications to function properly with other
   chips.
   https://github.com/Andy4495/ICM7218

   Interface requires up to 12 digital out pins for full support of
   all functionality. However, it is possible to use as few as 6 output
   pins if only a single decoded character set is needed.
*/
/* Version History
   1.0.0    02/12/2018  A.T.   Original
   1.1.0    02/27/2018  A.T.   Added support for ascii to segment mapping
   1.2.0    08/07/2018  A.T.   Changed names of shutdown() and wakeup() to
                               displayShutdown() and displayWakeup()
                               Renamed segment_map to ICM7218_segment_map
*/

/* Constructor:
      D0 - D3   : digital output data pins. D0 is least significant bit.
      D4        : digital output data pin. Also used for /SHUTDOWN mode
                  Use ICM7218::NO_PIN if pin is not connected to Arduino
      D5        : digital output data pin. Also used for /DECODE mode
                  Use ICM7218::NO_PIN if pin is not connected to Arduino
      D6        : digital output data pin. Also used for HEX/CODEB select
                  Use ICM7218::NO_PIN if pin is not connected to Arduino
      D7        : digital output data pin. Also used for DATA COMING signal
                  Use ICM7218::NO_PIN if pin is not connected to Arduino
      mode_pin  : digital output pin for MODE signal
      RF_pin    : digital output pin for /WRITE signal
*/
#include "ICM7218.h"

ICM7218::ICM7218(byte D0_pin, byte D1_pin, byte D2_pin, byte D3_pin,
                      byte D4_pin, byte D5_pin, byte D6_pin, byte D7_pin,
                      byte mode_pin, byte write_pin)
{
  d0_out =             D0_pin;
  d1_out =             D1_pin;
  d2_out =             D2_pin;
  d3_out =             D3_pin;
  d4_out =             D4_pin;         // /SHUTDOWN
  d5_out =             D5_pin;         // /DECODE
  d6_out =             D6_pin;         // HEXA (1) / CODEB (0)
  d7_out =             D7_pin;         // DATA COMING
  mode_out =           mode_pin;
  write_out =          write_pin;      // Active low

  digitalWrite(write_out, HIGH);  // Make sure /WRITE signal is inactive
  pinMode(write_out, OUTPUT);

  // Data pin levels don't need to be set in constructor, since they are only
  // latched by ICM7218B when the /WRITE signal is low.
  pinMode(d0_out, OUTPUT);
  pinMode(d1_out, OUTPUT);
  pinMode(d2_out, OUTPUT);
  pinMode(d3_out, OUTPUT);
  if (d4_out != NO_PIN) pinMode(d4_out, OUTPUT);
  if (d5_out != NO_PIN) pinMode(d5_out, OUTPUT);
  if (d6_out != NO_PIN) pinMode(d6_out, OUTPUT);
  if (d7_out != NO_PIN) pinMode(d7_out, OUTPUT);
  pinMode(mode_out, OUTPUT);

  mode = CODEB;            // Default mode is CODEB decode until changed with setMode()
  power_state = WAKEUP;    // Default power state is active until changed with shutdown()

}

void ICM7218::setMode(CHAR_MODE m) {
  switch (m) {
    case HEXA:
      decode_bit = 0;
      hexa_codeb_bit = 1;
      break;
    case CODEB:
      decode_bit = 0;
      hexa_codeb_bit = 0;
      break;
    case DIRECT:
      decode_bit = 1;
      hexa_codeb_bit = 0;
      break;
    default:
      decode_bit = 0;
      hexa_codeb_bit = 0;
      break;
  }
  // If current mode is DIRECT, and new mode is HEXA, then need to
  // re-send DIRECT control word with HEXA bit to avoid CODEB flash on LEDs
  if ( (mode == DIRECT) && (m == HEXA) )
    send_control(NO_DATA_COMING, hexa_codeb_bit, 1, power_state);

  mode = m;
}

void ICM7218::print(const char* s) {
  byte outbuf[9]; // Extra byte in case there is a leading decimal point (which does not get displayed)
  int index = 8;
  int i = 0;

    switch (mode) {
      case HEXA:
        memset(outbuf, 0 | DP, 9); // Initialize to default characters (0)
        while (index > 0) {
          switch (s[i]) {
              case '0': case '1': case '2': case '3':  case '4':
              case '5': case '6': case '7': case '8':  case '9':
                outbuf[--index] = (s[i] - '0' ) | DP;
                break;
              case 'A':  case 'B': case 'C': case 'D': case 'E': case 'F':
                outbuf[--index] = (s[i] - 'A' + 10) | DP;
                break;
              case 'a':  case 'b': case 'c': case 'd': case 'e': case 'f':
                outbuf[--index] = (s[i] - 'a' + 10) | DP;
                break;
              case '.':
                outbuf[index] = outbuf[index] & ~DP;
                break;
              case '\0':      // End of string
                index = 0;    // This will end the while loop
                break;
              default:        // Invalid character, use default character (0)
                --index;
                break;
            }
          i++;
        }
        // Check for a trailing decimal point
        if (s[i] == '.') outbuf[index] = outbuf[index] & ~DP;
        break;

    case CODEB:
      memset(outbuf, 15 | DP, 9); // Initialize to default characters (<space>)
      while (index > 0) {
        switch (s[i]) {
            case '0':  case '1': case '2': case '3': case '4':
            case '5':  case '6': case '7': case '8': case '9':
              outbuf[--index] = (s[i] - '0' ) | DP;
              break;
            case 'E':  case 'e':
              outbuf[--index] = 11 | DP;
              break;
            case 'H': case 'h':
              outbuf[--index] = 12 | DP;
              break;
            case 'L': case 'l':
              outbuf[--index] = 13 | DP;
              break;
            case 'P': case 'p':
              outbuf[--index] = 14 | DP;
              break;
            case '-':
              outbuf[--index] = 10 | DP;
              break;
            case ' ':
              outbuf[--index] = 15 | DP;
              break;
            case '.':
              outbuf[index] = outbuf[index] & ~DP;
              break;
            case '\0':     // End of string
              index = 0;   // This will end the while loop
              break;
            default:       // Invalid character printed as a blank
              --index;
              break;
          }
        i++;
      }
      // Check for a trailing decimal point
      if (s[i] == '.') outbuf[index] = outbuf[index] & ~DP;
      break;

    case DIRECT:
      memset(outbuf, 0 | DP, 9); // Initialize to default characters (0)
      for (i = 0; i < 8; i++) {
        if (s[i] != '\0') outbuf[7-i] = s[i];  // Flip the bytes around MSB<->LSB
        else break;  // Exit for loop once we hit end of string
      }
      break;

    default: // Send all dashes for invalid mode
      for (i = 0; i < 8; i++)
        outbuf[i] = 10;
      break;
  }
  // Set the mode
  send_control(DATA_COMING, hexa_codeb_bit, decode_bit, power_state);
  // Send the data
  for (i = 0; i < 8; i++) {
    send_byte(outbuf[i]);
  }
}

void ICM7218::displayShutdown() {
  power_state = SHUTDOWN;
  // Send control word, no data coming, with /SHUTDOWN active
  send_control(NO_DATA_COMING, hexa_codeb_bit, decode_bit, power_state);
}

void ICM7218::displayWakeup() {
  power_state = WAKEUP;
  /// Send control word, no data coming, with /SHUTDOWN inactive
  send_control(NO_DATA_COMING, hexa_codeb_bit, decode_bit, power_state);
}

#ifdef ICM7218_SEGMENT_MAP
void ICM7218::convertToSegments(char* s){
  int i = 0;
  int outindex = 0;
  int EOS = 0;    // end-of-string flag
  /// TO DO:
  ///  - Check for end-of-string before element 8
  ///  - Check for decimal point
  while (outindex < 8) {
      if (EOS == 0) {
        switch (s[i]) {
          case '.':
            if (outindex != 0)
              s[outindex-1] = s[outindex-1] & ~DP;             // Add decimal point
            i++;
            break;
          case '\0':   // End-of-string so fill with blanks
            s[outindex] = 0;
            i++;
            outindex++;
            EOS = 1;   // Turn on end-of-string flag
            break;
          default:
            // First 32 bytes ascii characters are non-printable, so
            // automatically display blank without defining in table
            if (s[i] < 32)
              s[outindex] = 0 | DP;
            else
            // Strip off MSB of input character since we are using 7-bit ascii
            // and set msb of output char to turn off decimal point
              s[outindex] = ICM7218_segment_map[(s[i] & 0x7f) - 32] | DP;
            i++;
            outindex++;
            break;
        }
      }
      else {        // Fill the remaining string with blank characters
        s[outindex] = 0 | DP;
        outindex++;
      }
  }
}
#endif

#ifdef ICM7218_SEGMENT_MAP
char ICM7218::convertToSegments(char c) {
  if (c < 32) return 0 | DP;    // Non-printable control characters
  else return ICM7218_segment_map[(c & 0x7f) - 32] | DP;
}
#endif

void ICM7218::send_byte(byte c) {
  // Change mode to DATA
  digitalWrite(mode_out, LOW);

  // Set output pins to data value
  digitalWrite(d0_out,     c  & 0x01);
  digitalWrite(d1_out, (c>>1) & 0x01);
  digitalWrite(d2_out, (c>>2) & 0x01);
  digitalWrite(d3_out, (c>>3) & 0x01);
  if (d4_out != NO_PIN) digitalWrite(d4_out, (c>>4) & 0x01);
  if (d5_out != NO_PIN) digitalWrite(d5_out, (c>>5) & 0x01);
  if (d6_out != NO_PIN) digitalWrite(d6_out, (c>>6) & 0x01);
  if (d7_out != NO_PIN) digitalWrite(d7_out, (c>>7) & 0x01);

  // Latch in the data
  digitalWrite(write_out, LOW);
  digitalWrite(write_out, HIGH);
}

void ICM7218::send_control(int dc, int hc, int decode, int sd) {
  // Setup control word bits
  if (d7_out != NO_PIN) digitalWrite(d7_out, dc);       // DATA_COMING
  if (d6_out != NO_PIN) digitalWrite(d6_out, hc);       // HEXA (1) / CODEB (0)
  if (d5_out != NO_PIN) digitalWrite(d5_out, decode);   // /DECODE
  if (d4_out != NO_PIN) digitalWrite(d4_out, sd);       // /SHUTDOWN

  // Latch in the bits
  digitalWrite(mode_out, HIGH);
  digitalWrite(write_out, LOW);    // Latch in the control bits
  digitalWrite(write_out, HIGH);
}
