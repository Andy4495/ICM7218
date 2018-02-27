ICM7218 Library
====================

This library to designed to interface with the Intersil/Renesas ICM7218A/B LED
driver chip. Note that there are several chips in the 72XX LED driver family,
including chips from Maxim.
These other chips provide similar capabilities; however, this library
was specifically designed for the Intersil/Renesas ISM7218A/B variants.

Support for other LED driver chips may be added in the future.

The I/O interface requires 10 digital output pins for full support of
all functionality. However, it is possible to use as few as 6 output
pins if only a single character set is needed. See "Reducing Output Pin Usage"
below for more info.

Usage
-----

Use the constructor to set up the pins used to interface with the 7218 chip.
A total of 10 output pins are required. For example,

    ICM7218 myLED(2, 3, 4, 5, 6, 7, 8, 9, 14, 15);

The first 8 parameters correspond to output data lines ID0 - ID7.
The ninth parameter is for the MODE pin and the final parameter is for the
WRITE pin.

Once you have created an ICM7218 object, the following methods can be
used to control the LEDs:

    void print(char* s)
  Sends the null-terminated character string s to the LED display.
  A maximum of 8 characters are printed (not including decimal points),
  regardless of the actual length of the string.
  Invalid characters (not supported by the current character mode) are
  displayed with a default character: '0' in HEXA mode or ' ' (space) in CODEB mode.
  If the string is less than 8 characters in length, then the string is left-justified
  and right-padded with the relevant default character (space or zero).

  Either upper- or lower-case alpha characters may be used in the string.
  However, the 7218 will display them the same regardless of case.

    void setMode(CHAR_MODE);
  Sets the character decode mode for the display. Three modes are supported:

- ICM7218::HEXA - Supports hexadecimal digits 0-9 and A-F, plus decimal points
- ICM7218::CODEB - Supports 0-9, H, E, L, P, - (hyphen), ' ' (space), plus decimal points (default mode)
- ICM7218::DIRECT - Direct control of LED segments. See [datasheet](https://www.intersil.com/content/dam/Intersil/documents/icm7/icm7218.pdf)
for bit-to-segment mapping.


    void shutdown();
  Turns off the display and puts the chip in low-power mode.
  The chip will accept new characters while in shutdown mode, so it is possible to use the
  print() method to "pre-display" a new string before calling wakeup().

    void wakeup();
  Brings the chip out of low-power mode and turns the display on.
  This is the default state when creating an ICM7218 object.

Reducing Output Pin Usage
-------------------------

It is possible to save up to four output pins by hardwiring some or all of the
pins ID4 - ID7 high or low to hardcode the character/segment decode mode and
shutdown mode.

Keep in mind, however, that ID7 is used for the decimal point, so if it is
tied high, then you won't be able access the decimal points in the LED display.

For example, to hardwire the chip into HEXA decode mode and always
have the display enabled, wire the pins as follows:

      ID7 (Data Coming) -> +5V
      ID6 (HEXA/CODEB)  -> +5V
      ID5 (DECODE)      -> GND
      ID4 (SHUTDOWN)    -> +5V

When invoking the constructor, use the value "ICM7218::NO_PIN" for any pins
that are hardwired. Using the above example:

      ICM7218 myLED(2, 3, 4, 5, ICM7218::NO_PIN, ICM7218::NO_PIN,
                    ICM7218::NO_PIN, ICM7218::NO_PIN, 14, 15);

References
----------
+ [ICM7218 datasheet](https://www.intersil.com/content/dam/Intersil/documents/icm7/icm7218.pdf)
