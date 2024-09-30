# ICM7218 Library

[![Arduino Compile Sketches](https://github.com/Andy4495/ICM7218/actions/workflows/arduino-compile-sketches.yml/badge.svg)](https://github.com/Andy4495/ICM7218/actions/workflows/arduino-compile-sketches.yml)
[![Check Markdown Links](https://github.com/Andy4495/ICM7218/actions/workflows/check-links.yml/badge.svg)](https://github.com/Andy4495/ICM7218/actions/workflows/check-links.yml)
[![Arduino Lint](https://github.com/Andy4495/ICM7218/actions/workflows/arduino-lint.yml/badge.svg)](https://github.com/Andy4495/ICM7218/actions/workflows/arduino-lint.yml)

This library is designed to interface with the Intersil/Renesas/Maxim ICM7218 and ICM7228 LED driver chips. This library should work with any of the variants (A, B, C, D) of the chips, although it has only been tested on the B and D (common cathode) variants.

## Quick Start

The library has several easy to use methods to quickly get a sketch running with basic functionality, while providing an interface which can take advantage of all the chips' features.

These examples demonstrate some common usage modes of the 72x8 chips. A complete description of the [public methods](#public-methods) supported by the library are further below. Be sure to also review the example sketches included with the library.

```cpp
// Intersil 7218A/B sequential update using HEXA decoding
#include "ICM7218.h"
ICM7218 myLED(2, 3, 4, 5, 6, 7, 8, 9, 10, 11); // Update per your configuration
void setup() {
  myLED.setMode(ICM7218::HEXA);
  myLED = "1234ABCD";
  myLED.print();
}

void loop() {
}
```

```cpp
// Intersil 7218C/D sequential update using HEXA mode
// Note the extra parameter in the constructor
#include "ICM7218.h"
ICM7218 myLED(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 1); // Update per your configuration
void setup() {
  myLED.setMode(ICM7218::HEXA);
  myLED = "1234ABCD";
  myLED.print();
}

void loop() {
}
```

```cpp
/// Intersil 7218A/B sequential update using DIRECT mode
#include "ICM7218.h"
ICM7218 myLED(2, 3, 4, 5, 6, 7, 8, 9, 10, 11); // Update per your configuration
void setup() {
  myLED.setMode(ICM7218::DIRECT);
  myLED = "1234ABCD";
  myLED.convertToSegments();
  myLED.print();
}

void loop() {
}
```

```cpp
/// Single digit update - not supported on Intersil 7218A/B
#include "ICM7218.h"
ICM7218 myLED(2, 3, 4, 5, 6, 7, 8, 9, 10, 11); // Update per your configuration
void setup() {
  myLED.setMode(ICM7218::HEXA);
  myLED = "1234ABCD";
  myLED.print();
  myLED.print('9', 0); // Change the left-most digit '1' to '9'
}

void loop() {
}
```

```cpp
/// Decimal points with CODEB mode
#include "ICM7218.h"
ICM7218 myLED(2, 3, 4, 5, 6, 7, 8, 9, 10, 11); // Update per your configuration
void setup() {
  myLED.setMode(ICM7218::CODEB);
  myLED = "12345678";
  myLED.dots = 0x08;  // Define a decimal point after the 5th digit 
  myLED.print(); // Display '12345.678'
}

void loop() {
}
```

```cpp
/// Decimal points with DIRECT mode
#include "ICM7218.h"
ICM7218 myLED(2, 3, 4, 5, 6, 7, 8, 9, 10, 11); // Update per your configuration
void setup() {
  myLED.setMode(ICM7218::DIRECT);
  myLED = "12345678";
  myLED.convertToSegments();
  myLED[3] = myLED[3] & 0x7F;   // Add a decimal point after the 4th digit 
  myLED.print(); // Display '1234.5678'
}

void loop() {
}
```

## Usage Details

### Display Layout

THe 72x8 driver chips define "DIGIT1" as the right-most digit in the display, and "DIGIT8" as the left-most digit. When using c-string char arrays with this library, array element `[0]` corresponds to DIGIT8 and array element `[7]` corresponds to DIGIT1. When using Single Digit Update mode, this library also uses the array element numbering with position 0 corresponding to the left-most digit:

|                           |   |   |   |   |   |   |   |   |
| ------------------------- | - | - | - | - | - | - | - | - |
| 72x8 DIGIT#               | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
| char array element        | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| single digit update       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |

If using a display with fewer than 8 digits, I would recommend "left-justifying" the digits with respect to the 72x8 driver chip. For example, with a 4-digit display, connected DIGIT8 to the left-most display digit, DIGIT7 to the next digit from left, DIGIT6 to the third digit, and DIGIT5 to the right-most display digit.

### Decimal Points

Handling the decimal points in displays is a little  complicated. Configuring the decimal points depends on how you are using the display.

#### With `print(char* s)` (only works with A or B variants of the chip)

This method takes in a c-string of ASCII characters.

For HEXA and CODEB modes, just include the decimal points in the string that you pass to `print(char*)`:

```cpp
myLED.setMode(ICM7218::HEXA);   // Or myLED.setMode(ICM7218::CODEB)
myLED.print("123.45678"); 
myLED.print("1.2.3.4.5.6.7.8."); // Any or all of the decimal points can be turned on
```

When using DIRECT mode, include the decimal points in the string that you pass `convertToSegments(char*)`:

```cpp
char buffer[17];
myLED.setMode(ICM7218::DIRECT);
strncpy(buffer, "123.45.678", 16);  // None, any, or all decimal points can be turned on
myLED.convertToSegments(buffer);
myLED.print()
```

#### With `print()` (no argument version; works with all variants of chip)

This method uses an internal class array to control the display digits (accessed with the array operator `[]` and the assignment operator `=`).

To turn on decimal points when using HEXA or CODEB modes, use the `.dots` data member. The most significant bit of `.dots` corresponds to the left-most display digit.

```cpp
myLED.setMode(ICM7218::HEXA);   // Or myLED.setMode(ICM7218::CODEB)
myLED = "12345678";
myLED.dots = 0x80;     // Turn on decimal point on the most significant (left-most) digit
myLED.print();         // Displays "1.2345678"
```

To turn on decimal points when using DIRECT mode, clear the most significant bit of the character after it is converted to segments. (This may seem counter-intuitive, but DIRECT mode sends the data as-is to the driver chip, and it expects bit 7 cleared to turn the decimal point on.):

```cpp
myLED.setMode(ICM7218::DIRECT);
myLED = "12345678"; 
myLED.convertToSegments();
myLED[1] = myLED[1] & 0x7f; // Turn on 2nd decimal point
myLED.print();
```

#### With `print(byte c, byte pos)`

To turn on decimal points when using HEXA or CODEB modes, use the `.dots` data member:

```cpp
myLED.setMode(ICM7218::HEXA);    // Or myLED.setMode(ICM7218::CODEB)
myLED = "12345678"; 
myLED.dots = 0;         // Turn off all decimal points
myLED.print();          // Displays "12345678"
// Note that the `.dots` bit position must match the one you are updating
myLED.dots = 0x80;      // Turn on the first (left-most) decimal point
myLED.print(9, 0);      // Displays "9.2345678"
```

To turn on decimal points when using DIRECT mode, clear the most significant bit of the character after it is converted to segments:

```cpp
char c;
myLED.setMode(ICM7218::DIRECT);
myLED = "12345678"; 
myLED.convertToSegments();
myLED.print();     // Displays "12345678"
c = convertToSegments('0')
c = c & 0x7f; // Clear the most significant bit
myLED.print(c, 4); // Displays "123.40678
```

### Public Methods

- `ICM7218 myLED(ID0, ID1, ID2, ID3, ID4, ID5, ID6, ID7, mode, write)`

Constructor for the A or B versions of the chip. Parameters are the pin numbers for ID0 through ID7, plus the mode and write pins.

- `ICM7218 myLED(ID0, ID1, ID2, ID3, ID7, DA0, DA1, DA2, mode, write, chip_cd)`

Constructor for the C or D versions of the chip. Has one additional parameter, which can be any 8-bit value (this parameter is used to differentiate between the two constructors, but the actual value passed does not matter to the library).

- `void print(char* s)`

Sends the character string `s` to the LED display. **This method only works with the A or B variants of the chips.**

A maximum of 8 characters are printed (not including decimal points), regardless of the actual length of the string. In HEXA or CODEB mode, if the string is less than 8 characters in length, then the string is left-justified and right-padded with the relevant default character (space or zero). In DIRECT mode, 8 characters are always printed (meaning memory locations beyond the end of the string may be accessed).

In HEXA or CODEB mode, either upper- or lower-case alpha characters may be used in the string and nvalid characters not supported by the current character mode are displayed with a default character: `'0'` (zero) in HEXA mode or `' '` (space) in CODEB mode.

In DIRECT mode, `s` is a sequence of bit-mapped bytes corresponding to specific display segments. See the `convertToSegments()` methods below.

- `void print()`

Sends the internal character array (accessed with the array operator `[]` or the assignment operator `=`) to the display.

- `void print(byte c, byte pos)`

Sends the single character `c` to the display at position `pos`. Display postions are numbered from 0 to 7 with the left-most digit being position 0.

For HEXA and CODEB modes, this method uses the `.dots` data member to determine if a decimal point is displayed with the updated digit.

For DIRECT mode, this is a bit-mapped segment value, including bit 7 representing the decimal point.

This method has no effect on chips that do not support Single Digit Update mode.

- `void covertToSegments(char* s)`

Converts the null-terminated ASCII string `s` to bit-mapped segment values used in DIRECT mode. The string is modified in-place. Exactly 8 display characters (not including decimal points) will be converted. Periods are converted into decimal points. See [below][3] for more details.

**WARNING: The string `s` passed to the method must be at least 8 bytes long, or unexpected results will occur.**

- `char convertToSegments(char c)`

Converts single character `c` (ASCII) and returns a bit-mapped segment value used in DIRECT mode. See [below][3] for more details.

- `void convertToSegments()`

Converts the internal character array (accessed with the array operator `[]` or the assignment operator `=`) to bit-mapped segment values used in DIRECT mode. The internal array is modified in-place, so any subsequent changes to the internal character array need to re-set the ASCII values and re-run `convertToSegments()`. See [below][3] for more details.

- `void setMode(CHAR_MODE)`

Sets the character decode mode for the display. `CHAR_MODE` is one of the following:

`ICM7218::HEXA` - Supports hexadecimal digits 0-9 and A-F, plus decimal points  
`ICM7218::CODEB` - Supports 0-9, H, E, L, P, - (hyphen), ' ' (space), plus decimal points (default mode)  
`ICM7218::DIRECT` - Direct control of LED segments. Each bit represents one LED segment plus the decimal point. See the [datasheet][4] for details on which bit corresponds to which segment. The convertToSegments() methods listed above provide a built-in mapping of ASCII characters to LED characters. See "Using Direct Mode" [below][3] for more information.

[3]: #using-direct-mode

- `void displayShutdown()`

Turns off the display and puts the chip in low-power mode. The chip will accept new characters while in shutdown mode, so it is possible to use the print() method to "pre-display" a new string before calling wakeup().

- `void displayWakeup()`

Brings the chip out of low-power mode and turns the display on. This is the default state when creating an `ICM7218` object.

- `void setBank(RAM_BANK)`

Sets the RAM bank to use with the next `print()` command. Only has effect on chips that support the feature. `RAM_BANK` can be either `ICM7218::RAM_BANK_A` or `ICM7218::RAM_BANK_B`.

- `operator []` and `operator =`

The `ICM7218` class provides a simplified interface by using an internal character array which can be accessed with the array index operator `[]` and the assignment operator `=`. This internal character array is used with the zero-argument `print()` and `convertToSegments()` methods. The library automatically does bounds checking. Attempts to write beyond the end of the internal array with operator `[]` will update the last byte of the array instead. Operator `=` will only copy the first 8 bytes of the assigned value.

## Using Direct Mode

DIRECT mode is only valid with A or B variants of the chip.

DIRECT mode allows direct control of each of the LED segments as opposed to the HEXA or CODEB modes which convert a numeric value into a corresponding LED digit or letter.

With DIRECT mode, each bit in the data byte directly corresponds to an LED segment or the decimal point. This mode is often used to control LED matrixes, but can also be used to display special characters and a range of alphabetic characters on a 7-segment display.

It is possible to use Figure 3 and Table 1 from the [datasheet][4] to manually determine the bit encoding to create the characters that you want to display. However, this library has a built-in mapping table and methods that can be used to convert ASCII characters to the corresponding LED segment bit values.

Use one of the `convertToSegments()` methods to convert ASCII characters to the DIRECT mode byte format.

**The sketches in the examples folder create different display strings both manually and using `convertToSegments()` for DIRECT mode.**

The ASCII to 7-segment mapping performed by `convertToSegments()` is shown in the following images. Note that the 7-segment display does not allow an accurate rendering of all ASCII characters and symbols. Also note that some values are rendered as a blank character, represented by a green box around the LED digit.

- ASCII 0x00 - 0x1F: Blank character (these are all non-printing ASCII control characters)
- ASCII 0x20 - 0x27: `spc ! " # $ % & '` ![0x20-0x27][20]
- ASCII 0x28 - 0x2F: `( ) * + , - . /` ![0x28-0x2F][28]
- ASCII 0x30 - 0x37: `0 1 2 3 4 5 6 7` ![0x30-0x37][30]
- ASCII 0x38 - 0x3F: `8 9 : ; < = > ?` ![0x38-0x3F][38]
- ASCII 0x40 - 0x47: `@ A B C D E F G` ![0x40-0x47][40]
- ASCII 0x48 - 0x4F: `H I J K L M N O` ![0x48-0x4F][48]
- ASCII 0x50 - 0x57: `P Q R S T U V W` ![0x50-0x57][50]
- ASCII 0x58 - 0x5F: `X Y Z [ \ ] ^ _` ![0x58-0x5F][58]
- ASCII 0x60 - 0x67: `` ` a b c d e f g`` ![0x60-0x67][60]
- ASCII 0x68 - 0x6F: `h i j k l m n o` ![0x68-0x6F][68]
- ASCII 0x70 - 0x77: `p q r s t u v w` ![0x70-0x77][70]
- ASCII 0x78 - 0x7F: `x y z { | } ~ DEL` ![0x78-0x7F][78]

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

### A and B variants

It is possible to save up to four output pins by hardwiring some or all of the pins ID4 - ID7 high or low to hardcode the character/segment decode mode and shutdown mode.

Keep in mind, however, that ID7 is used for the decimal point, so if it is tied high, then you won't be able access the decimal points in the LED display.

For example, to hardwire the chip into HEXA decode mode and always have the display enabled, wire the pins as follows:

```text
  ID7 (Data Coming) -> +5V
  ID6 (HEXA/CODEB)  -> +5V
  ID5 (DECODE)      -> GND
  ID4 (SHUTDOWN)    -> +5V
```

When invoking the constructor, use the value `ICM7218::NO_PIN` for any pins that are hardwired. Using the above example:

```cpp
  ICM7218 myLED(2, 3, 4, 5, ICM7218::NO_PIN, ICM7218::NO_PIN,
                ICM7218::NO_PIN, ICM7218::NO_PIN, 14, 15);
```

### C or D variants

The HEXA/CODEB/SHUTDOWN (mode) pin can be tied high or left floating to save a digital output pin. If tied high, then all digits will be displayed using HEXA decoding. If left floating, then all digits will be displayed using CODEB decoding.

When invoking the constructor, use the value `ICM7218::NO_PIN` for the `mode_pin` parameter.

## Reducing RAM Usage

When using HEXA or CODEB decoding exclusively, it is possible to save 192 bytes of RAM by disabling the `convertToSegments()` functionality. Add the following `#define` before including `ICM7218.h` in your sketch:

```cpp
#define ICM7218_NO_SEGMENT_MAP
```

## Other Notes

Beginning with version 1.3.0 of the library, Single Digit Update mode and RAM bank selection is supported (MAXIM ICM7218A/B/C/D and Intersil/Renesas ICM7218C/D and ICM7228A/B/C).

While the ICM7228 uses a 5V supply voltage, its data input lines are 3.3 V compatible. This means that it is possible to interface the ICM7228 with 3.3 V devices including MSP430 and 3.3V Arduino controllers without the need for level shifters.

The C and D variants of the chip combine the decode mode and wakeup/shutdown mode selection onto a single pin by using a three-state input:

| Pin 9 Level | Decode Mode | Power Mode |
| ----------- | ----------- | ---------- |
| High        | HEXA        | Wakeup     |
| Floating    | CODEB       | Wakeup     |
| Low         | N/A         | Shutdown   |

Because of the special configuration of this pin, a high level is represented by at least 4.2 V (other digital pins only need a minimum of 2 V to indicate a high level). This means that a 3.3 V microcontroller cannot configure the chip to use HEXA decoding without additional circuitry or externally pulling pin 9 to +5 V.

## References

- Intersil/Renesas [ICM7218 datasheet][4]
- Intersil/Renesas [ICM7228 datasheet][5]
- Maxim [ICM7218 datasheet][6]
- Alternate [library][7] specifically for ICM7218C chip by GitHub user [tttmmmsss][8]

## License

The software and other files in this repository are released under what is commonly called the [MIT License][100]. See the file [`LICENSE.txt`][101] in this repository.

[4]: https://www.renesas.com/us/en/document/dst/icm7218-datasheet
[5]: https://www.renesas.com/us/en/document/dst/icm7228-datasheet
[6]: https://www.analog.com/media/en/technical-documentation/data-sheets/ICM7218-ICM7228.pdf
[7]: https://github.com/tttmmmsss/ICM7218C
[8]: https://github.com/tttmmmsss
[100]: https://choosealicense.com/licenses/mit/
[101]: ./LICENSE.txt
[//]: # ([200]: https://github.com/Andy4495/ICM7218)
[//]: # ([6]: https://datasheets.maximintegrated.com/en/ds/ICM7218-ICM7228.pdf)
