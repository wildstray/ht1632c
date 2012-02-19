#include <avr/io.h> 
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include "Print.h"

#undef abs
#include <stdlib.h>

#define ht1632c_lib
#include <ht1632c.h>
#include "font_b.h"
#include "font_koi8.h"

/* fast integer (1 byte) modulus */

byte _mod(byte n, byte d)
{
  while(n >= d)
    n -= d;

  return n;
}

/* fast integer (1 byte) division */

byte _div(byte n, byte d)
{
  byte q = 0;
  while(n >= d)
  {
    n -= d;
    q++;
  }
  return q;
}

/* fast integer (1 byte) PRNG */

byte _rnd(byte min, byte max)
{
  static byte seed;
  seed = (21 * seed + 21);
  return min + _mod(seed, --max);
}

/* ht1632c class constructor */

ht1632c::ht1632c(const byte geometry, const byte number)
{
  if (geometry != GEOM_32x16) return;
  bicolor = true;
  x_max = (32 * number) - 1;
  y_max = 15;
  cs_max = 4 * number;
  fb_size = 16 * cs_max;
  g_fb = (byte*) malloc(fb_size);
  r_fb = (byte*) malloc(fb_size);
  setfont(FONT_5x7W);
  x_cur = 0;
  y_cur = 0;

  _data_out();
  _wr_out();
  _clk_out();
  _cs_out();
  setup();
}

/* send a command to selected HT1632Cs */

void ht1632c::sendcmd(byte cs, byte command)
{
  _chipselect(cs);
  _writebits(HT1632_ID_CMD, HT1632_ID_LEN);
  _writebits(command, HT1632_CMD_LEN);
  _writebits(0, 1);
  _chipselect(HT1632_CS_NONE);
}

/* HT1632Cs based display initialization  */

void ht1632c::setup()
{
  noInterrupts();
  sendcmd(HT1632_CS_ALL, HT1632_CMD_SYSDIS);
  sendcmd(HT1632_CS_ALL, HT1632_CMD_COMS00);
  sendcmd(HT1632_CS_ALL, HT1632_CMD_MSTMD);
  sendcmd(HT1632_CS_ALL, HT1632_CMD_RCCLK);
  sendcmd(HT1632_CS_ALL, HT1632_CMD_SYSON);
  sendcmd(HT1632_CS_ALL, HT1632_CMD_LEDON);
  sendcmd(HT1632_CS_ALL, HT1632_CMD_PWM);
  interrupts();
}

/* set the display brightness */ 

void ht1632c::pwm(byte value)
{
  sendcmd(HT1632_CS_ALL, HT1632_CMD_PWM | value);
}

/* write the framebuffer to the display - to be used after one or more textual or graphic functions */ 

void ht1632c::sendframe()
{
  byte offs, cs, csm;
  word addr;
  csm = cs_max;

  noInterrupts();
  for (cs = 0; cs < csm; cs++)
  {
    addr = cs + (cs & ~1) - (csm - 2) * _div((cs << 1), csm);
    addr <<= 4;
    _chipselect(cs + 1);
    _writebits(HT1632_ID_WR, HT1632_ID_LEN);
    _writebits(0, HT1632_ADDR_LEN);
    for (offs = 0; offs < 16; offs++)
      _writebits(g_fb[addr+offs], HT1632_DATA_LEN);
    for (offs = 0; offs < 16; offs++)
      _writebits(r_fb[addr+offs], HT1632_DATA_LEN);
    _chipselect(HT1632_CS_NONE);
  }
  interrupts();
}

/* clear the display */

void ht1632c::clear()
{
  x_cur = 0;
  y_cur = 0;
  memset(g_fb, 0, fb_size);
  memset(r_fb, 0, fb_size);
  sendframe();
}

void ht1632c::update_framebuffer(byte *ptr, byte target, byte pixel) 
{
  byte &val = *ptr;
  val |= pixel;
  if (!target) 
    val ^= pixel;
}

/* put a single pixel in the coordinates x, y */

void ht1632c::plot (byte x, byte y, byte color)
{
  byte val;
  word addr;

  if (x > x_max || y > y_max)
    return;

  addr = x + cs_max * (y & ~7);
  val = 128 >> (y & 7);
  update_framebuffer(g_fb+addr, (color & GREEN), val);
  update_framebuffer(r_fb+addr, (color & RED), val); 
}

/* print a single character */

byte ht1632c::putchar(int x, int y, char c, byte color, byte attr, byte bgcolor)
{
  word dots, msb;
  char col, row;

  byte width = font_width;
  byte height = font_height;

  if (x < -width || x > x_max + width || y < -height || y > y_max + height)
    return 0;

  if ((unsigned char) c >= 0xc0) c -= 0x41;
  c -= 0x20; 
  msb = 1 << (height - 1);

  // some math with pointers... don't try this at home ;-)
  prog_uint8_t *addr = font + c * width;
  prog_uint16_t *waddr = wfont + c * width;

  for (col = 0; col < width; col++) {
    dots = (height > 8) ? pgm_read_word_near(waddr + col) : pgm_read_byte_near(addr + col);
    for (row = 0; row < height; row++) {
      if (dots & (msb >> row)) {
        plot(x + col, y + row, (color & MULTICOLOR) ? _rnd(1, 4) : color);
      } else {
        plot(x + col, y + row, bgcolor);
      }
    }
  }
  return ++width;
}

/* put a bitmap in the coordinates x, y */

void ht1632c::putbitmap(int x, int y, prog_uint16_t *bitmap, byte w, byte h, byte color)
{
  if (x < -w || x > x_max + w || y < -h || y > y_max + h)
    return;

  byte col, row;

  word msb = 1 << (w - 1);
  for (row = 0; row < h; row++)
  {
    uint16_t dots = pgm_read_word_near(&bitmap[row]);
    if (dots && color)
      for (col = 0; col < w; col++)
      {
	if (dots & (msb >> col))
	  plot(x + col, y + row, color);
	else
	  plot(x + col, y + row, BLACK);
      }
  }
}

/* text only scrolling functions */

void ht1632c::hscrolltext(int y, char *text, byte color, int delaytime, int times, byte dir, byte attr, byte bgcolor)
{
  byte showcolor;
  int x, offs, len = strlen(text);
  byte width = font_width;
  byte height = font_height;
  width++;

  while (times) {
    for ((dir) ? x = - (len * width) : x = x_max; (dir) ? x <= x_max : x > - (len * width); (dir) ? x++ : x--)
    {
      for (int i = 0; i < len; i++)
      {
        offs = width * i;
        (dir) ? offs-- : offs++;
        putchar(x + offs,  y, text[i], bgcolor, attr, bgcolor);
      }
      for (int i = 0; i < len; i++)
      {
        showcolor = (color & RANDOMCOLOR) ? _rnd(1, 4) : color;
        showcolor = ((color & BLINK) && (x & 2)) ? BLACK : (showcolor & ~BLINK);
        offs = width * i;
        putchar(x + offs,  y, text[i], showcolor, attr, bgcolor);
      }
      sendframe();
      delay(delaytime);
    }
    times--;
  }
}

void ht1632c::vscrolltext(int x, char *text, byte color, int delaytime, int times, byte dir, byte attr, byte bgcolor)
{
  byte showcolor;
  int y, voffs, len = strlen(text);
  byte offs;
  byte width = font_width;
  byte height = font_height;
  width++;

  while (times) {
    for ((dir) ? y = - font_height : y = y_max + 1; (dir) ? y <= y_max + 1 : y > - font_height; (dir) ? y++ : y--)
    {
      for (int i = 0; i < len; i++)
      {
        offs = width * i;
        voffs = (dir) ? -1 : 1;
        putchar(x + offs,  y + voffs, text[i], bgcolor, attr, bgcolor);
      }
      for (int i = 0; i < len; i++)
      {
        showcolor = (color & RANDOMCOLOR) ? _rnd(1, 4) : color;
        showcolor = ((color & BLINK) && (y & 2)) ? BLACK : (showcolor & ~BLINK);
        putchar(x + width * i,  y, text[i], showcolor, attr, bgcolor);
      }
      sendframe();
      delay(delaytime);
    }
    times--;
  }
}

/* choose/change font to use (for next putchar) */

void ht1632c::setfont(byte userfont)
{
  switch(userfont) {

#ifdef FONT_4x6
    case FONT_4x6:
	font = (prog_uint8_t *) &font_4x6[0];
	font_width = 4;
	font_height = 6;
	break;
#endif
#ifdef FONT_5x7
    case FONT_5x7:
	font = (prog_uint8_t *) &font_5x7[0];
	font_width = 5;
	font_height = 7;
	break;
#endif
#ifdef FONT_5x8
    case FONT_5x8:
	font = (prog_uint8_t *) &font_5x8[0];
	font_width = 5;
	font_height = 8;
	break;
#endif
#ifdef FONT_5x7W
    case FONT_5x7W:
	font = (prog_uint8_t *) &font_5x7w[0];
	font_width = 5;
	font_height = 8;
	break;
#endif
#ifdef FONT_6x10
    case FONT_6x10:
	wfont = (prog_uint16_t *) &font_6x10[0];
	font_width = 6;
	font_height = 10;
	break;
#endif
#ifdef FONT_6x12
    case FONT_6x12:
	wfont = (prog_uint16_t *) &font_6x12[0];
	font_width = 6;
	font_height = 12;
	break;
#endif
#ifdef FONT_6x13
    case FONT_6x13:
	wfont = (prog_uint16_t *) &font_6x13[0];
	font_width = 6;
	font_height = 13;
	break;
#endif
#ifdef FONT_6x13B
    case FONT_6x13B:
	wfont = (prog_uint16_t *) &font_6x13B[0];
	font_width = 6;
	font_height = 13;
	break;
#endif
#ifdef FONT_6x13O
    case FONT_6x13O:
	wfont = (prog_uint16_t *) &font_6x13O[0];
	font_width = 6;
	font_height = 13;
	break;
#endif
#ifdef FONT_6x9
    case FONT_6x9:
	wfont = (prog_uint16_t *) &font_6x9[0];
	font_width = 6;
	font_height = 9;
	break;
#endif
#ifdef FONT_7x13
    case FONT_7x13:
	wfont = (prog_uint16_t *) &font_7x13[0];
	font_width = 7;
	font_height = 13;
	break;
#endif
#ifdef FONT_7x13B
    case FONT_7x13B:
	wfont = (prog_uint16_t *) &font_7x13B[0];
	font_width = 7;
	font_height = 13;
	break;
#endif
#ifdef FONT_7x13O
    case FONT_7x13O:
	wfont = (prog_uint16_t *) &font_7x13O[0];
	font_width = 7;
	font_height = 13;
	break;
#endif
#ifdef FONT_7x14
    case FONT_7x14:
	wfont = (prog_uint16_t *) &font_7x14[0];
	font_width = 7;
	font_height = 14;
	break;
#endif
#ifdef FONT_7x14B
    case FONT_7x14B:
	wfont = (prog_uint16_t *) &font_7x14B[0];
	font_width = 7;
	font_height = 14;
	break;
#endif
#ifdef FONT_8x8
    case FONT_8x8:
	font = (prog_uint8_t *) &font_8x8[0];
	font_width = 8;
	font_height = 8;
	break;
#endif
#ifdef FONT_8x13
    case FONT_8x13:
	wfont = (prog_uint16_t *) &font_8x13[0];
	font_width = 8;
	font_height = 13;
	break;
#endif
#ifdef FONT_8x13B
    case FONT_8x13B:
	wfont = (prog_uint16_t *) &font_8x13B[0];
	font_width = 8;
	font_height = 13;
	break;
#endif
#ifdef FONT_8x13O
    case FONT_8x13O:
	wfont = (prog_uint16_t *) &font_8x13O[0];
	font_width = 8;
	font_height = 13;
	break;
#endif
#ifdef FONT_9x15
    case FONT_9x15:
	wfont = (prog_uint16_t *) &font_9x15[0];
	font_width = 9;
	font_height = 15;
	break;
#endif
#ifdef FONT_9x15B
    case FONT_9x15B:
	wfont = (prog_uint16_t *) &font_9x15b[0];
	font_width = 9;
	font_height = 15;
	break;
#endif
#ifdef FONT_8x16
    case FONT_8x16:
	wfont = (prog_uint16_t *) &font_8x16[0];
	font_width = 8;
	font_height = 16;
	break;
#endif
#ifdef FONT_8x16B
    case FONT_8x16B:
	wfont = (prog_uint16_t *) &font_8x16b[0];
	font_width = 8;
	font_height = 16;
	break;
#endif
#ifdef FONT_8x13BK
    case FONT_8x13BK:
	wfont = (prog_uint16_t *) &font_8x13bk[0];
	font_width = 8;
	font_height = 13;
	break;
#endif
  }
}

/* graphic primitives based on Bresenham's algorithms */

void ht1632c::line(int x0, int y0, int x1, int y1, byte color)
{
  int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
  int err = dx + dy, e2; /* error value e_xy */

  for(;;) {
    plot(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}

void ht1632c::rect(int x0, int y0, int x1, int y1, byte color)
{
  line(x0, y0, x0, y1, color); /* left line   */
  line(x1, y0, x1, y1, color); /* right line  */
  line(x0, y0, x1, y0, color); /* top line    */
  line(x0, y1, x1, y1, color); /* bottom line */
}

void ht1632c::circle(int xm, int ym, int r, byte color)
{
  int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */ 
  do {
    plot(xm - x, ym + y, color); /*   I. Quadrant */
    plot(xm - y, ym - x, color); /*  II. Quadrant */
    plot(xm + x, ym - y, color); /* III. Quadrant */
    plot(xm + y, ym + x, color); /*  IV. Quadrant */
    r = err;
    if (r >  x) err += ++x * 2 + 1; /* e_xy+e_x > 0 */
    if (r <= y) err += ++y * 2 + 1; /* e_xy+e_y < 0 */
  } while (x < 0);
}

void ht1632c::ellipse(int x0, int y0, int x1, int y1, byte color)
{
  int a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1; /* values of diameter */
  long dx = 4 * (1 - a) * b * b, dy = 4 * (b1 + 1) * a * a; /* error increment */
  long err = dx + dy + b1 * a * a, e2; /* error of 1.step */

  if (x0 > x1) { x0 = x1; x1 += a; } /* if called with swapped points */
  if (y0 > y1) y0 = y1; /* .. exchange them */
  y0 += (b + 1) / 2; /* starting pixel */
  y1 = y0 - b1;
  a *= 8 * a; 
  b1 = 8 * b * b;

  do {
    plot(x1, y0, color); /*   I. Quadrant */
    plot(x0, y0, color); /*  II. Quadrant */
    plot(x0, y1, color); /* III. Quadrant */
    plot(x1, y1, color); /*  IV. Quadrant */
    e2 = 2 * err;
    if (e2 >= dx) { x0++; x1--; err += dx += b1; } /* x step */
    if (e2 <= dy) { y0++; y1--; err += dy += a; }  /* y step */ 
  } while (x0 <= x1);

  while (y0 - y1 < b) {  /* too early stop of flat ellipses a=1 */
    plot(x0 - 1, ++y0, color); /* -> complete tip of ellipse */
    plot(x0 - 1, --y1, color); 
  }
}

void ht1632c::bezier(int x0, int y0, int x1, int y1, int x2, int y2, byte color)
{
  int sx = x0 < x2 ? 1 : -1, sy = y0 < y2 ? 1 : -1; /* step direction */
  int cur = sx * sy * ((x0 - x1) * (y2 - y1) - (x2 - x1) * (y0 - y1)); /* curvature */
  int x = x0 - 2 * x1 + x2, y = y0 - 2 * y1 + y2, xy = 2 * x * y * sx * sy;
                                /* compute error increments of P0 */
  long dx = (1 - 2 * abs(x0 - x1)) * y * y + abs(y0 - y1) * xy - 2 * cur * abs(y0 - y2);
  long dy = (1 - 2 * abs(y0 - y1)) * x * x + abs(x0 - x1) * xy + 2 * cur * abs(x0 - x2);
                                /* compute error increments of P2 */
  long ex = (1 - 2 * abs(x2 - x1)) * y * y + abs(y2 - y1) * xy + 2 * cur * abs(y0 - y2);
  long ey = (1 - 2 * abs(y2 - y1)) * x * x + abs(x2 - x1) * xy - 2 * cur * abs(x0 - x2);

  if (cur == 0) { line(x0, y0, x2, y2, color); return; } /* straight line */

  x *= 2 * x; y *= 2 * y;
  if (cur < 0) {                             /* negated curvature */
    x = -x; dx = -dx; ex = -ex; xy = -xy;
    y = -y; dy = -dy; ey = -ey;
  }
  /* algorithm fails for almost straight line, check error values */
  if (dx >= -y || dy <= -x || ex <= -y || ey >= -x) {        
    line(x0, y0, x1, y1, color);                /* simple approximation */
    line(x1, y1, x2, y2, color);
    return;
  }
  dx -= xy; ex = dx+dy; dy -= xy;              /* error of 1.step */

  for(;;) {                                         /* plot curve */
    plot(x0, y0, color);
    ey = 2 * ex - dy;                /* save value for test of y step */
    if (2 * ex >= dx) {                                   /* x step */
      if (x0 == x2) break;
      x0 += sx; dy -= xy; ex += dx += y; 
    }
    if (ey <= 0) {                                      /* y step */
      if (y0 == y2) break;
      y0 += sy; dx -= xy; ex += dy += x; 
    }
  }
}

/* returns the pixel value (RED, GREEN, ORANGE or 0/BLACK) of x, y coordinates */

byte ht1632c::getpixel(byte x, byte y)
{
  byte g, r, val;
  word addr;

  addr = x + cs_max * (y & ~7);
  val = 128 << (y & 7);
  g = g_fb[addr];
  r = r_fb[addr];
  return (g & val) ? GREEN : BLACK | (r & val) ? RED : BLACK;
}

/* boundary flood fill with the seed in x, y coordinates */

void ht1632c::fill_r(byte x, byte y, byte color)
{
  if (x > x_max) return;
  if (y > y_max) return;
  if(!getpixel(x, y))
  {
    plot(x, y, color);
    fill_r(++x, y ,color);
    x--;
    if (x > x_max) return;
    fill_r(x, y - 1, color);
    fill_r(x, y + 1, color);
  }
}

void ht1632c::fill_l(byte x, byte y, byte color)
{
  if (x > x_max) return;
  if (y > y_max) return;
  if(!getpixel(x, y))
  {
    plot(x, y, color);
    fill_l(--x, y, color);
    x++;
    fill_l(x, y - 1, color);
    fill_l(x, y + 1, color);
  }
}

void ht1632c::fill(byte x, byte y, byte color)
{
  fill_r(x, y, color);
  fill_l(x - 1, y, color);
}

/* Print class extension: TBD */

size_t ht1632c::write(uint8_t chr)
{
  byte x, y;
  if (chr == '\n') {
    //y_cur += font_height;
  } else {
    //x_cur += putchar(x_cur, y_cur, chr, GREEN);
    //x_cur = 0;
    //y_cur = 0;
  }
  //sendframe();
  return 1;
}

size_t ht1632c::write(const char *str)
{
  byte x, y;
  byte len = strlen(str);
  if (x_cur + font_width <= x_max)
    for (byte i = 0; i < len; i++)
    {
      if (str[i] == '\n') {
        y_cur += font_height;
        x_cur = 0;
        if (y_cur > y_max) break;
      } else {
        x_cur += putchar(x_cur, y_cur, str[i], GREEN);
        if (x_cur + font_width > x_max) break;
      }
    }
  //x_cur = 0;
  //y_cur = 0;
  sendframe();
  return len;
}

/* calculate frames per second speed, for benchmark */

void ht1632c::profile() {
  const byte frame_interval = 50;
  static unsigned long last_micros = 0;
  static byte frames = 0;
  if (++frames == frame_interval) {
    fps = (frame_interval * 1000000) / (micros() - last_micros);
    frames = 0;
    last_micros = micros();
  }
}
