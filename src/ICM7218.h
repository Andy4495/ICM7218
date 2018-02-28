/* Library to interface with Intersil/Renesas ICM7218A/B LED driver chip.
   Note that there are several chips in the 72xx family, including
   chips from Maxim.
   While these other chips provide similar capabilities, this library
   was specifically designed for the Intersil ISM7218A/B variants.
   This library may require modifications to function properly with other
   chips.
   https://gitlab.com/Andy4495/ICM7218

   Interface requires up to 12 digital out pins for full support of
   all functionality. However, it is possible to use as few as 6 output
   pins if only a single decoded character set is needed.
*/
/* Version History
   1.0.0    02/12/2018  A.T.   Original
*/
#ifndef ICM7218_LIBRARY
#define ICM7218_LIBRARY

#include <Arduino.h>

class ICM7218 {
public:
  enum CHAR_MODE {HEXA=1, CODEB=0, DIRECT=2};
  enum {NO_PIN=255};
  ICM7218(byte D0, byte D1, byte D2, byte D3, byte D4, byte D5, byte D6,
          byte D7, byte mode_pin, byte write_pin);
  void setMode(CHAR_MODE);
  void print(const char*);
  void shutdown();
  void wakeup();


private:
  enum POWER_MODE {WAKEUP = 1, SHUTDOWN = 0};
  enum {DP = 128};
  enum {NO_DATA_COMING = 0, DATA_COMING = 1};
  byte d0_out, d1_out, d2_out, d3_out, d4_out, d5_out, d6_out, d7_out;
  byte mode_out;
  byte write_out;
  int mode, decode_bit, hexa_codeb_bit;
  int power_state;
  void send_byte(byte b);
  void send_control(int dc, int hc, int decode, int sd);
};
#endif
