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
   1.1.0    02/27/2018  A.T.   Added ascii to segment mapping table
   1.2.0    08/07/2018  A.T.   Changed names of shutdown() and wakeup() to
                               displayShutdown() and displayWakeup()
                               Moved segment_map out of class definition and
                               renamed to ICM7218_segment_map
*/
#ifndef ICM7218_LIBRARY
#define ICM7218_LIBRARY

// Save 192 byts of RAM by defining ICM7218_NO_SEGMENT_MAP before #including this header
#ifndef ICM7218_NO_SEGMENT_MAP
#define ICM7218_SEGMENT_MAP
#endif

#include <Arduino.h>

class ICM7218 {
public:
  enum CHAR_MODE {HEXA=1, CODEB=0, DIRECT=2};
  enum {NO_PIN=255};
  enum {DP = 128};
  ICM7218(byte D0, byte D1, byte D2, byte D3, byte D4, byte D5, byte D6,
          byte D7, byte mode_pin, byte write_pin);
  void setMode(CHAR_MODE);
  void print(const char*);
  void displayShutdown();
  void displayWakeup();
#ifdef ICM7218_SEGMENT_MAP
  void convertToSegments(char*);
  char convertToSegments(char);
#endif

private:
  enum POWER_MODE {WAKEUP = 1, SHUTDOWN = 0};
  enum {NO_DATA_COMING = 0, DATA_COMING = 1};
  byte d0_out, d1_out, d2_out, d3_out, d4_out, d5_out, d6_out, d7_out;
  byte mode_out;
  byte write_out;
  int mode, decode_bit, hexa_codeb_bit;
  int power_state;
  void send_byte(byte b);
  void send_control(int dc, int hc, int decode, int sd);
};

#ifdef ICM7218_SEGMENT_MAP
  // Conversion of letters to LED segments.
  // 0x00 is a blank character and is used for unsupported values.
  // Save memory by not defining first 32 ascii characters, since they
  // are all control characters
  const unsigned char ICM7218_segment_map[96]  = {
  //  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00: Control characters
  //  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 08: Control characters
  //  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 10: Control characters
  //  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 18: Control characters
    0x00, 0x67, 0x22, 0x41, 0x18, 0x12, 0x45, 0x20, // 20: spc ! " # $ % & '
    0x49, 0x51, 0x63, 0x28, 0x10, 0x04, 0x00, 0x2c, // 28:   ( ) * + , - . /
    0x7b, 0x30, 0x6d, 0x75, 0x36, 0x57, 0x5f, 0x70, // 30:   0 1 2 3 4 5 6 7
    0x7f, 0x77, 0x44, 0x5d, 0x0d, 0x05, 0x15, 0x6c, // 38:   8 9 : ; < = > ?
    0x00, 0x7e, 0x1f, 0x4b, 0x3d, 0x4f, 0x4e, 0x5b, // 40:   @ A B C D E F G
    0x3e, 0x0a, 0x39, 0x0f, 0x0b, 0x5c, 0x1c, 0x7b, // 48:   H I J K L M N O
    0x6e, 0x76, 0x0c, 0x57, 0x4a, 0x3b, 0x3b, 0x59, // 50:   P Q R S T U V W
    0x3a, 0x37, 0x7d, 0x4b, 0x16, 0x71, 0x62, 0x01, // 58:   X Y Z [ \ ] ^ _
    0x02, 0x7e, 0x1f, 0x0d, 0x3d, 0x4f, 0x4e, 0x5b, // 60:   ` a b c d e f g
    0x1e, 0x08, 0x39, 0x0f, 0x0b, 0x5c, 0x1c, 0x1d, // 68:   h i j k l m n o
    0x6e, 0x76, 0x0c, 0x57, 0x4a, 0x19, 0x19, 0x59, // 70:   p q r s t u v w
    0x3a, 0x37, 0x7d, 0x4d, 0x08, 0x55, 0x66, 0x00  // 78:   x y z { | } ~ DEL
  };
#endif

#endif
