## Library usage ##

In you project include the library header, and instantiate HT1632C class with correct port and pin numbers.

Arduino 1.0:

```
#include <ht1632c.h>

ht1632c ledMatrix = ht1632c(&PORTD, 7, 6, 4, 5, GEOM_32x16, 2);

```

Leaflab Maple, Chipkit Uno32, etc.:

```
#include <ht1632c.h>

ht1632c ledMatrix = ht1632c(7, 6, 4, 5, GEOM_32x16, 2);

```


Note: pin numbers could be relative (0~7) to port or absolute Arduino style pin numer (0~14, A0-A5) with the last version of the library.

With the library there are over 20 fonts included. You cannot fill flash memory compiling all provided fonts, so you must comment some font definitions in ht1632c.h:

```
//#define FONT_4x6     1
//#define FONT_5x7     2
#define FONT_5x8     3
#define FONT_5x7W    4
//#define FONT_6x10    5
//#define FONT_6x12    6
//#define FONT_6x13    7
//#define FONT_6x13B   8
//#define FONT_6x13O   9
//#define FONT_6x9    10
//#define FONT_7x13   11
//#define FONT_7x13B  12
//#define FONT_7x13O  13
//#define FONT_7x14   14
//#define FONT_7x14B  15
#define FONT_8x8    16
//#define FONT_8x13   17
//#define FONT_8x13B  18
//#define FONT_8x13O  19
#define FONT_9x15   20
#define FONT_9x15B  21
#define FONT_8x16   22
#define FONT_8x16B  23
```

### Control functions ###

```
void pwm(byte value);
void sendframe();
void clear();
```

**pwm()** sets the display brightness. Input value range is 0-15.

**sendframe()** writes buffer memory to display. It's required after one or more text of graphic function.

**clear()** clears buffer memory and display. It doesn't require sendframe().

### Text functions ###

```
byte putchar(int x, int y, char c, byte color = GREEN, byte attr = 0);
void hscrolltext(int y, const char *text, byte color, int delaytime, int times = 1, byte dir = LEFT);
void vscrolltext(int x, const char *text, byte color, int delaytime, int times = 1, byte dir = UP);
void setfont(byte userfont);
```

**putchar()** puts a character **c** at **x, y** coordinates of color **color** (defaults GREEN) with attributes **attr** (future use).

**hscrolltext()** and **vscrolltext()** does horizontal or vertical scroll of a text **text** of color **color** with a delay of **delaytime** milliseconds, and repeats **times** times, with direction **dir** (RIGHT/LEFT or UP/DOWN).

**setfont()** change font to **userfont**.

### Graphic functions ###

```
void plot (byte x, byte y, byte color);
byte getpixel (byte x, byte y);
void line(int x0, int y0, int x1, int y1, byte color);
void rect(int x0, int y0, int x1, int y1, byte color);
void circle(int xm, int ym, int r, byte color);
void ellipse(int x0, int y0, int x1, int y1, byte color);
void fill (byte x, byte y, byte color);
void bezier(int x0, int y0, int x1, int y1, int x2, int y2, byte color);
void putbitmap(int x, int y, prog_uint16_t *bitmap, byte w, byte h, byte color);
```

**plot()** put a pixel of color **color** in **x, y** coordinates.

**getpixel()** reads the color of the pixel in **x, y** coordinates.

**line()** draw a line from **x0, y0** to **x1, y1** of color **color**.

**rect()** draw a rectangle with vertexes in **x0, y0** and **x1, y1** of color **color**.

**circle()** draw a circle with center in **xm, ym** and radius of **r** of color **color**.

**ellipse()** draw an ellipse inscribed in the rectangle with vertexes in **x0, y0** and **x1, y1** of color **color**.

**bezier()** draw a bezier curve from **x0, y0** to **x2, y2** with direction **x1, y1** of color **color**.

**fill()** boundary flood fill of color **color** starting from **x, y**.

**putbitmap()** puts a bitmap of color **color**, of width **w** and height **h**, in the coordinates **x, y**. Bitmap is supposed to be > 8 pixel wide, stored in PROGMEM.

## Hardware documentation (HT1632C, AVR, etc.) ##

  * [HT1632C datasheet](http://www.holtek.com/pdf/consumer/ht1632cv120.pdf)
  * [74164 SIPO Shift Register datasheet](http://www.datasheetcatalog.org/datasheets/270/387544_DS.pdf)
  * [Sure Electronics DP14112 P4 32X16 RG datasheet](http://www.sure-electronics.net/download/DE-DP14112_Ver1.1_EN.pdf)
  * [Sure Electronics DP14211 P7.62 32X16 RG datasheet](http://www.sure-electronics.net/download/DE-DP14211_Ver1.1_EN.pdf)
  * [ATmega48A/PA/88A/PA/168A/PA/328/P datasheet](http://www.atmel.com/dyn/resources/prod_documents/doc8271.pdf)

## Helpful Arduino and AVR documentation and links ##

  * [Ardunio playground - HT1632C](http://www.arduino.cc/playground/Main/HT1632C)
  * [Arduino new forum - Sure Electronics new 32x16 bi-color display: 3216 RG](http://arduino.cc/forum/index.php/topic,50326.0.html)
  * [Arduino old forum - SureElectronics 3216 Led Matrix](http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1292604415/0)
  * [Arduino old forum - Arduino to 384 LED display module (MCU Interface)](http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1225239439/all)
  * [Arduino Language Reference](http://arduino.cc/en/Reference/HomePage)
  * [AVR libc](http://www.nongnu.org/avr-libc/)
  * [AVR035: Efficient C Coding for AVR](http://www.atmel.com/dyn/resources/prod_documents/doc1497.pdf)
  * [AVR Assembler tutorial](http://www.avr-asm-tutorial.net/avr_en/index.html)

## Helpful ChipKIT and PIC32 documentation and links ##

  * [Programming PIC32 in C](http://www.abramovbenjamin.net/orcad/Programming_PIC32_in_C.pdf)

## Helpful C language documentation ##
  * [The C Programming Language - Brian Kernighan and Dennis Ritchie, Second Edition](http://net.pku.edu.cn/~course/cs101/2008/resource/The_C_Programming_Language.pdf)
  * [Oxford University - Programming in C](http://www.oucs.ox.ac.uk/documentation/userguides/c/l922.pdf)
  * [C++ Language Tutorial](http://www.cplusplus.com/files/tutorial.pdf)
  * [Bresenham's algorithms for graphic primitives](http://free.pages.at/easyfilter/bresenham.html)

## Helpful documentation on SCMs ##
  * [Version control with Subversion](http://svnbook.red-bean.com/index.en.html)
  * [Controllo di versione con Subversion (Italian)](http://svnbook.red-bean.com/)
  * [Mercurial: The Definitive Guide](http://hgbook.red-bean.com/)
  * [Mercurial: la guida definitiva (Italian)](http://gpiancastelli.altervista.org/hgbook-it/)