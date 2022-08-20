# ICM7218 Library

[![Arduino Compile Sketches](https://github.com/Andy4495/ICM7218/actions/workflows/arduino-compile-sketches.yml/badge.svg)](https://github.com/Andy4495/ICM7218/actions/workflows/arduino-compile-sketches.yml)
[![Check Markdown Links](https://github.com/Andy4495/ICM7218/actions/workflows/CheckMarkdownLinks.yml/badge.svg)](https://github.com/Andy4495/ICM7218/actions/workflows/CheckMarkdownLinks.yml)
[![Arduino Lint](https://github.com/Andy4495/ICM7218/actions/workflows/arduino-lint.yml/badge.svg)](https://github.com/Andy4495/ICM7218/actions/workflows/arduino-lint.yml)

This library is designed to interface with the Intersil/Renesas ICM7218A/B and ICM7228A/B LED driver chips. Note that there are several chips in the 72XX LED driver family, including chips from Maxim. These other chips provide similar capabilities; however, this library was specifically designed for the Intersil/Renesas ICM7218A/B and ICM7228A/B variants.

On the ICM7228B chip, the library only supports "Sequential 8-Digit Update" mode. This means that the ICM7228 is functionally equivalent to the ICM7218 when using this library.

Support for other LED driver chips and additional control modes for ICM7218 may be added in the future.

The I/O interface requires 10 digital output pins for full support of
all functionality. However, it is possible to use as few as 6 output
pins if only a single character set is needed. See "Reducing Output Pin Usage" [below][1] for more info.

While the ICM7228 uses a 5V supply voltage, its data input lines are 3.3 V compatible. This means that it is possible to interface the ICM7228 with 3.3 V devices including MSP430 and 3.3V Arduino controllers without the need for level shifters.

[1]: #reducing-output-pin-usage

## Usage

Use the constructor to set up the pins used to interface with the 7218 chip. A total of 10 output pins are required. For example,

    ICM7218 myLED(2, 3, 4, 5, 6, 7, 8, 9, 14, 15);

The first 8 parameters correspond to output data lines ID0 - ID7. The ninth parameter is for the MODE pin and the final parameter is for the WRITE pin.

Once you have created an ICM7218 object, the following methods can be
used to control the LEDs:

    void print(char* s)
  Sends the null-terminated character string `s` to the LED display.

  A maximum of 8 characters are printed (not including decimal points),
  regardless of the actual length of the string. Invalid characters (not supported by the current character mode) are displayed with a default character: `'0'` (zero) in HEXA mode or `' '` (space) in CODEB mode. If the string is less than 8 characters in length, then the string is left-justified and right-padded with the relevant default character (space or zero).

  Either upper- or lower-case alpha characters may be used in the string.

    void setMode(CHAR_MODE);
  Sets the character decode mode for the display. `CHAR_MODE` is one of the following:

`ICM7218::HEXA` - Supports hexadecimal digits 0-9 and A-F, plus decimal points  
`ICM7218::CODEB` - Supports 0-9, H, E, L, P, - (hyphen), ' ' (space), plus decimal points (default mode)  
`ICM7218::DIRECT` - Direct control of LED segments. Each bit represents one LED segment plus the decimal point. See [datasheet][2] for details on which bit corresponds to which segment. The convertToSegments() methods listed below provide a built-in mapping of ASCII characters to LED characters. See "Using Direct Mode" [below][3] for more information.

[2]: https://www.intersil.com/content/dam/Intersil/documents/icm7/icm7218.pdf
[3]: #using-direct-mode

    void displayShutdown();
  Turns off the display and puts the chip in low-power mode. The chip will accept new characters while in shutdown mode, so it is possible to use the print() method to "pre-display" a new string before calling wakeup().

    void displayWakeup();
  Brings the chip out of low-power mode and turns the display on. This is the default state when creating an ICM7218 object.

    void covertToSegments(char* s);
  Converts the null-terminated ASCII string `s` to bit-mapped segment values used in DIRECT mode. The string is modified in-place. Only the first 8 characters (fewer if a null-terminator is detected) will be converted. See [below][3] for more details.

    char convertToSegments(char c);
  Converts single character `c` (ASCII) and returns a bit-mapped segment value used in DIRECT mode. See [below][3] for more details.

## Using Direct Mode

DIRECT mode allows direct control of each of the LED segments as opposed to the HEXA or CODEB modes which convert a numeric value into a corresponding LED digit or letter.

With DIRECT mode, each bit in the data byte directly corresponds to an LED segment or the decimal point. This mode is often used to control LED matrixes, but can also be used to display special characters and a range of alphabetic characters on a 7-segment display.

It is possible to use Figure 3 and Table 1 from the [datasheet][2] to manually determine the bit encoding to create the characters that you want to display. However, this library has a built-in mapping table and methods that can be used to convert ASCII characters to the corresponding LED segment bit values.

There are two versions of the `convertToSegments()` method. To convert a full null-terminated string, use `convertToSegments(char* s)`. This will convert the character string `s` to a byte array that can be used with `print(s)` in DIRECT mode. The string is converted in-place up to a maximum of 8 characters (less if a null-terminator is encountered). Any characters after the eighth (or a null-terminator) will be ignored and left as-is. Periods `.` are converted into decimal points.

To convert just one character, use `convertToSegments(char c)`, which returns a converted character which can them be placed in a string used with `print(s)`. The single-character version of this method will convert a period into a blank display character.

**The sketch in the examples folder creates different display strings both manually and using `convertToSegments()` for DIRECT mode.**

The ASCII to 7-segment mapping performed by `convertToSegments()` is shown in the following images. Note that the 7-segment display does not allow an accurate rendering of all ASCII characters and symbols. Also note that some values are rendered as a blank character, represented by a green box around the LED digit.

* ASCII 0x00 - 0x1F: Blank character (these are all non-printing ASCII control characters)
* ASCII 0x20 - 0x27: `spc ! " # $ % & '` ![0x20-0x27][20]
* ASCII 0x28 - 0x2F: `( ) * + , - . /` ![0x28-0x2F][28]
* ASCII 0x30 - 0x37: `0 1 2 3 4 5 6 7` ![0x30-0x37][30]
* ASCII 0x38 - 0x3F: `8 9 : ; < = > ?` ![0x38-0x3F][38]
* ASCII 0x40 - 0x47: `@ A B C D E F G` ![0x40-0x47][40]
* ASCII 0x48 - 0x4F: `H I J K L M N O` ![0x48-0x4F][48]
* ASCII 0x50 - 0x57: `P Q R S T U V W` ![0x50-0x57][50]
* ASCII 0x58 - 0x5F: `X Y Z [ \ ] ^ _` ![0x58-0x5F][58]
* ASCII 0x60 - 0x67: `` ` a b c d e f g`` ![0x60-0x67][60]
* ASCII 0x68 - 0x6F: `h i j k l m n o` ![0x68-0x6F][68]
* ASCII 0x70 - 0x77: `p q r s t u v w` ![0x70-0x77][70]
* ASCII 0x78 - 0x7F: `x y z { | } ~ DEL` ![0x78-0x7F][78]

[20]: extras/jpg/ascii20-27.jpg "ASCII 0x20-0x27"
[28]: extras/jpg/ascii28-2F.jpg "ASCII 0x28-0x2F"
[30]: extras/jpg/ascii30-37.jpg "ASCII 0x30-0x37"
[38]: extras/jpg/ascii38-3F.jpg "ASCII 0x38-0x3F"
[40]: extras/jpg/ascii40-47.jpg "ASCII 0x40-0x47"
[48]: extras/jpg/ascii48-4F.jpg "ASCII 0x48-0x4F"
[50]: extras/jpg/ascii50-57.jpg "ASCII 0x50-0x57"
[58]: extras/jpg/ascii58-5F.jpg "ASCII 0x58-0x5F"
[60]: extras/jpg/ascii60-67.jpg "ASCII 0x60-0x67"
[68]: extras/jpg/ascii68-6F.jpg "ASCII 0x60-0x6F"
[70]: extras/jpg/ascii70-77.jpg "ASCII 0x70-0x77"
[78]: extras/jpg/ascii78-7F.jpg "ASCII 0x78-0x7F"

## Reducing Output Pin Usage

It is possible to save up to four output pins by hardwiring some or all of the pins ID4 - ID7 high or low to hardcode the character/segment decode mode and shutdown mode.

Keep in mind, however, that ID7 is used for the decimal point, so if it is tied high, then you won't be able access the decimal points in the LED display.

For example, to hardwire the chip into HEXA decode mode and always have the display enabled, wire the pins as follows:

      ID7 (Data Coming) -> +5V
      ID6 (HEXA/CODEB)  -> +5V
      ID5 (DECODE)      -> GND
      ID4 (SHUTDOWN)    -> +5V

When invoking the constructor, use the value `ICM7218::NO_PIN` for any pins that are hardwired. Using the above example:

      ICM7218 myLED(2, 3, 4, 5, ICM7218::NO_PIN, ICM7218::NO_PIN,
                    ICM7218::NO_PIN, ICM7218::NO_PIN, 14, 15);

## Reducing RAM Usage

It is possible to save 192 bytes of RAM by disabling the `convertToSegments()` functionality. Add the following `#define` before including `ICM7218.h` in your sketch:

    #define ICM7218_NO_SEGMENT_MAP

## References

* [ICM7218 datasheet](https://www.intersil.com/content/dam/Intersil/documents/icm7/icm7218.pdf)

## License

The software and other files in this repository are released under what is commonly called the [MIT License][100]. See the file [`LICENSE.txt`][101] in this repository.

[100]: https://choosealicense.com/licenses/mit/
[101]: ./LICENSE.txt
[200]: https://github.com/Andy4495/ICM7218
