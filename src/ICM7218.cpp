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

/* Constructor to use with the A or B variants of the chip.
   Ten required parameters:
     ID0_pin - ID3_pin : digital output data pins. D0 is least significant bit.
     ID4               : digital output data pin. Also used for /SHUTDOWN mode
                         Use ICM7218::NO_PIN if pin is not connected to Arduino
     ID5               : digital output data pin. Also used for /DECODE mode
                         Use ICM7218::NO_PIN if pin is not connected to Arduino
     ID6               : digital output data pin. Also used for HEX/CODEB select
                         Use ICM7218::NO_PIN if pin is not connected to Arduino
     ID7               : digital output data pin. Also used for DATA COMING signal
                         Use ICM7218::NO_PIN if pin is not connected to Arduino
     mode_pin          : digital output pin for MODE signal
     write_pin         : digital output pin for /WRITE signal
*/
ICM7218::ICM7218(byte ID0_pin, byte ID1_pin, byte ID2_pin, byte ID3_pin,
                 byte ID4_pin, byte ID5_pin, byte ID6_pin, byte ID7_pin,
                 byte mode_pin, byte write_pin)
{
  d0_out =             ID0_pin;
  d1_out =             ID1_pin;
  d2_out =             ID2_pin;
  d3_out =             ID3_pin;
  d4_out =             ID4_pin;        // /SHUTDOWN
  d5_out =             ID5_pin;        // /DECODE
  d6_out =             ID6_pin;        // HEXA (1) / CODEB (0)
  d7_out =             ID7_pin;        // DATA COMING
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
  ab_or_cd = CHIP_AB;
} // Constructor for A or B chip variant

/* Constructor to use with the C or D variants of the chip
   Eleven required parameters:
     ID0_pin - ID3_pin : digital output data pins. ID0 is least significant bit
     ID7_pin           : decimal point data, active low
     DA0_pin - DA2_pin : digit address pins. DA0 is least significant bit
     mode_pin          : digital output pin for HEXA/CODEB/SHUTDOWN signal
                         Use ICM7218::NO_PIN if pin is not connected to Arduino
     write_pin         : digital output pin for /WRITE signal
     chip_cd           : indicates that this is C or D variant of chip; the value does not matter
*/
ICM7218::ICM7218(byte ID0_pin, byte ID1_pin, byte ID2_pin, byte ID3_pin, byte ID7_pin,
                 byte DA0_pin, byte DA1_pin, byte DA2_pin, 
                 byte mode_pin, byte write_pin, byte chip_cd) {
  d0_out =             ID0_pin;
  d1_out =             ID1_pin;
  d2_out =             ID2_pin;
  d3_out =             ID3_pin;
  d4_out =             DA0_pin;        // Digit address lsb
  d5_out =             DA1_pin;        // Digit address 
  d6_out =             DA2_pin;        // Digit address msb
  d7_out =             ID7_pin;        // Decimal point
  mode_out =           mode_pin;       // HIGH = HEXA, Floating (input) = CODEB, LOW = SHUTDOWN
  write_out =          write_pin;      // Active low
  ab_or_cd =           CHIP_CD;

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
  if (mode_out != NO_PIN) {
    digitalWrite(mode_out, LOW);  // Make sure no pullup connected
    pinMode(mode_out, INPUT);   // Default is CODEB (floating) with Display Enabled
  }

  mode = CODEB;            // Default mode is CODEB decode until changed with setMode()
  power_state = WAKEUP;    // Default power state is active until changed with shutdown()
  ram_bank_select = RAM_BANK_A;   // Only useful on ICM7228
  ab_or_cd = CHIP_CD | (chip_cd & 0x01);  // Obfuscated code to avoid an "unused parameter" warning from compiler
} // Constructor for C or D variant

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
  if (ab_or_cd == CHIP_AB) {
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
  }
  else {  // C or D chip variant. No control word; update MODE pin.
    if (power_state == WAKEUP) {
      switch (m) {
        case HEXA: 
          if (mode_out != NO_PIN) {
            digitalWrite(mode_out, HIGH);
            pinMode(mode_out, OUTPUT);
          }
          break;
        case CODEB:  // floating
        default:     // Default mode is CODEB
          if (mode_out != NO_PIN) {
            digitalWrite(mode_out, LOW);
            pinMode(mode_out, INPUT);
          }
          break;
      }
    }
  }
  mode = m;
}

void ICM7218::setBank(RAM_BANK bs) {
  ram_bank_select = bs;
}

// This method only works with the A and B variants of the chip
void ICM7218::print(const char* s) {
  byte outbuf[MAX_DIGITS + 1]; // Extra byte in case there is a leading decimal point (which does not get displayed)
  int index = MAX_DIGITS;
  int i = 0;
  
  // This method only works with the A and B variants of the chip
  if (ab_or_cd == CHIP_AB) {
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
  }
} // print(const char*)

void ICM7218::print() {
  byte display_digit;
  if (ab_or_cd == CHIP_AB) {
    // Send control byte to start the transfer
    send_control(DATA_COMING, hexa_codeb_bit, decode_bit, power_state);
  }
  // Send the data bytes in reverse order
  for (int i = MAX_DIGITS - 1; i >= 0 ; i--) {
    switch (mode) {
      case HEXA:
        display_digit = convertToHexa(display_array[i]);
        display_digit |= ((dots<<i) & DP) ? 0 : DP;
        break; 
      case CODEB:
        display_digit = convertToCodeB(display_array[i]);
        display_digit |= ((dots<<i) & DP) ? 0 : DP;
        break;
      case DIRECT:
        display_digit = display_array[i];
        break;
      default: // Send all 0's if invalid mode. This should never happen!
        display_digit = 0;
        break; 
    }
    if (ab_or_cd == CHIP_AB) {
      send_byte(display_digit);
    }
    else { // C or D chip variants
      send_byte(display_digit, MAX_DIGITS - i - 1);
    }
  }
}  // print()

// For use with ICM7228 A/B Single Digit Update mode or ICM7218 C, D, ICM7228C update mode
// pos is the array position, not the DIGIT#. That is, pos = 0 refers to left-most digit
void ICM7218::print(byte c, byte pos) {
  if (pos > MAX_DIGITS - 1) pos = MAX_DIGITS - 1;
  switch (mode) {
    case HEXA:
      c = convertToHexa(c);
      c |= ((dots<<pos) & DP) ? 0 : DP;
      break; 
    case CODEB:
      c = convertToCodeB(c);
      c |= ((dots<<pos) & DP) ? 0 : DP;
      break;
    default: // Nothing to do for DIRECT mode
      break; 
  }  
  if (ab_or_cd == CHIP_AB) {
    send_control(NO_DATA_COMING, hexa_codeb_bit, decode_bit, power_state, MAX_DIGITS - pos - 1);
    send_byte(c);
  }
  else { // C or D chip variants
    send_byte(c, MAX_DIGITS - pos - 1);
  }
}  // print(char c, int pos)


void ICM7218::displayShutdown() {
  power_state = SHUTDOWN;
  if (ab_or_cd == CHIP_AB) {
    // Send control word, no data coming, with /SHUTDOWN active
    send_control(NO_DATA_COMING, hexa_codeb_bit, decode_bit, power_state);
  }
  else { // C or D chip variants
    if (mode_out != NO_PIN) {
      digitalWrite(mode_out, LOW);
      pinMode(mode_out, OUTPUT);
    }
  }
}

void ICM7218::displayWakeup() {
  power_state = WAKEUP;
  if (ab_or_cd == CHIP_AB) {
    /// Send control word, no data coming, with /SHUTDOWN inactive
    send_control(NO_DATA_COMING, hexa_codeb_bit, decode_bit, power_state);
  }
  else { // C or D chip variants
    if (mode == HEXA) {
      if (mode_out != NO_PIN) {
        digitalWrite(mode_out, HIGH);
        pinMode(mode_out, OUTPUT);
      }
    }
    else { // CODEB (floating)
      if (mode_out != NO_PIN) {
        digitalWrite(mode_out, LOW);
        pinMode(mode_out, INPUT);    
      }
    }
  }
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

// For use with A and B chip variants
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

// C and D variants write individual characters with 3 address bits
void ICM7218::send_byte(byte c, byte pos) {

  // Set output pins to data value
  digitalWrite(d0_out,     c  & 0x01);
  digitalWrite(d1_out, (c>>1) & 0x01);
  digitalWrite(d2_out, (c>>2) & 0x01);
  digitalWrite(d3_out, (c>>3) & 0x01);
  // For address pins, need to flip the bit because library addresses left to right
  digitalWrite(d4_out, (pos & 0x01) ? 0 : 1);  // DA0
  digitalWrite(d5_out, (pos & 0x02) ? 0 : 1);  // DA1
  digitalWrite(d6_out, (pos & 0x04) ? 0 : 1);  // DA2
  digitalWrite(d7_out, (c & 0x80) ? 0 : 1);

  // Latch in the data
  digitalWrite(write_out, LOW);
  digitalWrite(write_out, HIGH);
}

void ICM7218::send_control(byte dc, byte hc, byte decode, byte sd, byte addr) {
  // Setup control word bits
  if (d7_out != NO_PIN) digitalWrite(d7_out, dc);       // DATA_COMING
  if (d6_out != NO_PIN) digitalWrite(d6_out, hc);       // HEXA (1) / CODEB (0)
  if (d5_out != NO_PIN) digitalWrite(d5_out, decode);   // /DECODE
  if (d4_out != NO_PIN) digitalWrite(d4_out, sd);       // /SHUTDOWN
  digitalWrite(d3_out, ram_bank_select);  // Don't care for Intersil ICM7218, RAM bank select for ICM7228 and Maxim IMC7218
  digitalWrite(d2_out, addr & 0x04);
  digitalWrite(d1_out, addr & 0x02);
  digitalWrite(d0_out, addr & 0x01); 

  // Latch in the bits
  digitalWrite(mode_out, HIGH);
  digitalWrite(write_out, LOW);    // Latch in the control bits
  digitalWrite(write_out, HIGH);
}

byte ICM7218::convertToCodeB(byte c) {
  byte display_digit;
  switch (c) {
    case '0':  case '1': case '2': case '3': case '4':
    case '5':  case '6': case '7': case '8': case '9':
      display_digit = (c - '0' );
      break;
    case 'E':  case 'e':
      display_digit = 11;
      break;
    case 'H': case 'h':
      display_digit = 12;
      break;
    case 'L': case 'l':
      display_digit = 13;
      break;
    case 'P': case 'p':
      display_digit = 14;
      break;
    case '-':
      display_digit = 10;
      break;
    case ' ':
      display_digit = 15;
      break;
    default:       // Invalid character printed as a blank
      display_digit = 15;
      break;
  }
  return display_digit;
}

byte ICM7218::convertToHexa(byte c) {
  byte display_digit;
  switch (c) {
    case '0': case '1': case '2': case '3':  case '4':
    case '5': case '6': case '7': case '8':  case '9':
      display_digit = (c - '0' );
      break;
    case 'A':  case 'B': case 'C': case 'D': case 'E': case 'F':
      display_digit = (c - 'A' + 10);
      break;
    case 'a':  case 'b': case 'c': case 'd': case 'e': case 'f':
      display_digit = (c - 'a' + 10);
      break;
    default:        // Invalid character, use default character (0)
      display_digit = 0 | DP;
      break;
  }
  return display_digit;
}
