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
   1.3.0    08/24/2022  Andy4495 Add methods to simplify usage
*/

#include "ICM7218.h"

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
  ram_bank_select = RAM_BANK_A;   // Only useful on ICM7228

}

byte& ICM7218::operator [] (byte index) {
  if (index >= MAX_DIGITS) index = MAX_DIGITS - 1;
  return display_array[index];
}

byte ICM7218::operator [] (byte index) const {
  if (index >= MAX_DIGITS) index = MAX_DIGITS - 1;
  return display_array[index];
}

void ICM7218::operator= (const char * s) {
  memcpy(display_array, s, MAX_DIGITS);
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
  byte outbuf[MAX_DIGITS + 1]; // Extra byte in case there is a leading decimal point (which does not get displayed)
  int index = MAX_DIGITS;
  int i = 0;

    switch (mode) {
      case HEXA:
        memset(outbuf, 0 | DP, MAX_DIGITS + 1); // Initialize to default characters (0)
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
      memset(outbuf, 15 | DP, MAX_DIGITS + 1); // Initialize to default characters (<space>)
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
      memset(outbuf, 0 | DP, MAX_DIGITS + 1); // Initialize to default characters (0)
      for (i = 0; i < MAX_DIGITS; i++) {
        // Previous versions of this library stopped when '\0' was detected
        // However, '\0' is a valid value in DIRECT mode, so we should process it
        // Since this is a read-only operation, going beyond end of array will
        // not corrupt memory.
        outbuf[MAX_DIGITS - 1 - i] = s[i];  // Flip the bytes around MSB<->LSB
      }
      break;

    default: // Send all zeroes for invalid mode. THIS SHOULD NEVER HAPPEN!
      for (i = 0; i < MAX_DIGITS; i++) 
        outbuf[i] = 0;
      break;
  }
  // Set the mode
  send_control(DATA_COMING, hexa_codeb_bit, decode_bit, power_state);
  // Send the data
  for (i = 0; i < MAX_DIGITS; i++) {
    send_byte(outbuf[i]);
    // Copy the data sent to display into the object's internal storage
    display_array[MAX_DIGITS - i - 1] = outbuf[i];
  }
} // print(const char*)

void ICM7218::print() {
  byte display_digit;
  // Send control byte to start the transfer
  send_control(DATA_COMING, hexa_codeb_bit, decode_bit, power_state);
  // Send the data bytes in reverse order
  for (int i = MAX_DIGITS - 1; i >= 0 ; i--) {
    switch (mode) {
      case HEXA:
        switch (display_array[i]) {
          case '0': case '1': case '2': case '3':  case '4':
          case '5': case '6': case '7': case '8':  case '9':
            display_digit = (display_array[i] - '0' ) | DP;
            break;
          case 'A':  case 'B': case 'C': case 'D': case 'E': case 'F':
            display_digit = (display_array[i] - 'A' + 10) | DP;
            break;
          case 'a':  case 'b': case 'c': case 'd': case 'e': case 'f':
            display_digit = (display_array[i] - 'a' + 10) | DP;
            break;
          default:        // Invalid character, use default character (0)
            display_digit = 0 | DP;
            break;
        }
        break; 
      case CODEB:
        switch (display_array[i]) {
          case '0':  case '1': case '2': case '3': case '4':
          case '5':  case '6': case '7': case '8': case '9':
            display_digit = (display_array[i] - '0' ) | DP;
            break;
          case 'E':  case 'e':
            display_digit = 11 | DP;
            break;
          case 'H': case 'h':
            display_digit = 12 | DP;
            break;
          case 'L': case 'l':
            display_digit = 13 | DP;
            break;
          case 'P': case 'p':
            display_digit = 14 | DP;
            break;
          case '-':
            display_digit = 10 | DP;
            break;
          case ' ':
            display_digit = 15 | DP;
            break;
          default:       // Invalid character printed as a blank
            display_digit = 15 | DP;
            break;
        }
        break;
      case DIRECT:
        display_digit = display_array[i];
        break;
      default: // Send all 0's if invalid mode. This should never happen!
        display_digit = 0;
        break; 
    }
    send_byte(display_digit);
  }
}  // print()

// For use with ICM7228 Single Digit Update mode
void ICM7218::print(char c, byte pos) {
  if (pos > MAX_DIGITS - 1) pos = MAX_DIGITS - 1;
  send_control(NO_DATA_COMING, hexa_codeb_bit, decode_bit, power_state, ram_bank_select, pos);
  send_byte(c);
  display_array[pos] = c;
}  // print(char c, int pos)


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
/* Converts the ASCII character string s into the segment format used in DIRECT mode
   s is modified in place and must be at least 8 bytes long.    
*/
void ICM7218::convertToSegments(char* s){
  int i = 0;
  int outindex = 0;
  int EOS = 0;    // end-of-string flag
  /// TO DO:
  ///  - Check for decimal point on last character
  while (outindex < MAX_DIGITS) {
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
/* Converts the ASCII character c the segment format used in DIRECT mode
*/
char ICM7218::convertToSegments(char c) {
  if (c < 32) return 0 | DP;    // Non-printable control characters
  else return ICM7218_segment_map[(c & 0x7f) - 32] | DP;
}
#endif

#ifdef ICM7218_SEGMENT_MAP
/* Converts the ASCII character string stored in the object 
   into the segment format used in DIRECT mode
   The display object string is modified in place.   
   Display decimal points are not supported by this function since
   we are limited to the internal 8-byte storage
   Decimals can be added after calling this function by clearing
   bit 7 on the relevant digits.  
*/
void ICM7218::convertToSegments() {
  int i;
  for(i = 0; i < MAX_DIGITS; i++) {
    if (display_array[i] < 32)
      display_array[i] = 0 | DP;
    else
    // Strip off MSB of input character since we are using 7-bit ascii
    // and set msb of output char to turn off decimal point
      display_array[i] = ICM7218_segment_map[(display_array[i] & 0x7f) - 32] | DP;
  }
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

void ICM7218::send_control(byte dc, byte hc, byte decode, byte sd, byte bs, byte addr) {
  // Setup control word bits
  if (d7_out != NO_PIN) digitalWrite(d7_out, dc);       // DATA_COMING
  if (d6_out != NO_PIN) digitalWrite(d6_out, hc);       // HEXA (1) / CODEB (0)
  if (d5_out != NO_PIN) digitalWrite(d5_out, decode);   // /DECODE
  if (d4_out != NO_PIN) digitalWrite(d4_out, sd);       // /SHUTDOWN
  digitalWrite(d3_out, bs);  // Don't care for ICM7218, RAM bank select for ICM7228
  digitalWrite(d2_out, addr & 0x04);
  digitalWrite(d1_out, addr & 0x02);
  digitalWrite(d0_out, addr & 0x01); 

  // Latch in the bits
  digitalWrite(mode_out, HIGH);
  digitalWrite(write_out, LOW);    // Latch in the control bits
  digitalWrite(write_out, HIGH);
}
