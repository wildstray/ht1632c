#include <avr/io.h> 
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <WProgram.h>
#include "pins_arduino.h"
#include "font.h"
#include "font2.h"
#include "font3.h"

#undef abs
#include <stdlib.h>

#define ht1632c_lib
#include <ht1632c.h>
//#undef ht1632c

//void ht1632c::_clk_pulse(byte num)
/*
void _clk_pulse(byte num)
{
  while(num--)
  {
    _clk_set();
    _clk_clr();
  }
}
*/

ht1632c::ht1632c(const byte geometry, const byte number = 1)
{
  if (geometry != GEOM_32x16) return;
  bicolor = true;
  x_max = (32 * number) - 1;
  y_max = 15;
  cs_max = 4 * number;
  framesize = 32 * cs_max;
  framebuffer = (byte*) malloc(framesize);

  _data_out();
  _wr_out();
  _clk_out();
  _cs_out();
  setup();
}

/*
void ht1632class::begin(const byte data, const byte wr, const byte clk, const byte cs, byte geometry, byte number = 1)
{
  if (geometry != GEOM_32x16) return;
  bicolor = true;
  x_max = (32 * number) - 1;
  y_max = 15;
  cs_max = 4 * number;
  framesize = 32 * cs_max;
  framebuffer = (byte*) malloc(framesize);

  _data = _pin(data);
  _wr = _pin(wr);
  _clk = _pin(clk);
  _cs = _pin(cs);

  setup();
}
*/
/*
void ht1632class::begin(byte data, byte wr, byte cs1, byte cs2, byte cs3, byte cs4, byte geometry, byte number = 1)
{
  if ((geometry != GEOM_32x8) || (geometry != GEOM_24x16)) return;
  bicolor = false;
  cs_max = number;
  if (geometry == GEOM_32x8) { 
    framesize = 64 * cs_max; 
    x_max = (32 * number) - 1;
    y_max = 7;
  }
  if (geometry == GEOM_24x16) { 
    framesize = 96 * cs_max;
    x_max = (24 * number) - 1;
    y_max = 15;
  }
  framebuffer = (byte*) malloc(framesize);

  ht1632_data.pin = data;
  ht1632_wr.pin = wr;
  ht1632_clk.pin = 0;

  setup();
}
*/
/*
void ht1632c::chipselect(byte cs)
{
  if (cs == HT1632_CS_ALL) { // Enable all ht1632class
    _cs_clr();
    _clk_pulse(cs_max);
  } else if (cs == HT1632_CS_NONE) { //Disable all ht1632classs
    _cs_set(); 
    _clk_pulse(cs_max);
  } else {
    _cs_clr();
    _clk_pulse(1);
    _cs_set(); 
    _clk_pulse(cs - 1);
  }
}

void ht1632c::writebits (byte bits, byte msb)
{
  while (msb) {
    !!(bits & msb) ? _data_set() : _data_clr();
    _wr_clr();
    _wr_set();
    msb >>= 1;
  }
}
*/
void ht1632c::sendcmd (byte cs, byte command)
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
/*
  _out(_data);
  _out(_wr);
  _out(_clk);
  _out(_cs);
*/
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
  byte data, offs, cs;
  word addr;

  noInterrupts();
  for (cs = 0; cs < cs_max; cs++)
  {
    _chipselect(cs+1);
    _writebits(HT1632_ID_WR, HT1632_ID_LEN);
    _writebits(0, HT1632_ADDR_LEN);
    addr = cs + (cs & 2) - 2*!!(cs & 4);
    addr *= 16;
    for (offs = 0; offs < 16; offs++)
      _writebits(framebuffer[addr+offs], HT1632_DATA_LEN);
    for (offs = 0; offs < 16; offs++)
      _writebits(framebuffer[addr+offs+128], HT1632_DATA_LEN);
    _chipselect(HT1632_CS_NONE);
  }
  interrupts();
}

/* clear the display */

void ht1632c::clear()
{
  memset(framebuffer, 0, framesize);
  sendframe();
}

void ht1632c::update_framebuffer(word addr, byte target_bitval, byte pixel_bitval) 
{
  byte &v = framebuffer[addr];
  (target_bitval) ? v |= pixel_bitval : v &= ~pixel_bitval;
}

/* put a single pixel in the coordinates x, y */

void ht1632c::plot (byte x, byte y, byte color)
{
  byte bitval;
  word addr;

  if (x < 0 || x > x_max || y < 0 || y > y_max)
    return;

  if (color != BLACK && color != GREEN && color != RED && color != ORANGE)
    return;

  bitval = 128 >> (y & 7);
  addr = (x & 63) + 8*(y & ~7);

  update_framebuffer(addr, (color & GREEN), bitval);
  update_framebuffer(addr+128, (color & RED), bitval); 
}

/* print a single character */

void ht1632c::putchar(int x, int y, char c, byte color = GREEN)
{
  byte dots;
  //if (c >= 'A' && c <= 'Z' ||
  //  (c >= 'a' && c <= 'z') ) {
  //  c &= 0x1F;   // A-Z maps to 1-26
  //} 
  //else if (c >= '0' && c <= '9') {
  //  c = (c - '0') + 27;
  //} 
  //else if (c == ' ') {
  //  c = 0; // space
  //}

  if (c == ' ') {c = 0;}
  else if (c == '!') {c = 1;}
  else if (c == '"') {c = 2;}
  else if (c == '#') {c = 3;}
  else if (c == '$') {c = 4;}
  else if (c == '%') {c = 5;}
  else if (c == '&') {c = 6;}
  //else if (c == ''') {c = 7;}
  else if (c == '(') {c = 8;}
  else if (c == ')') {c = 9;}
  else if (c == '*') {c = 10;}
  else if (c == '+') {c = 11;}
  else if (c == ',') {c = 12;}
  else if (c == '-') {c = 13;}
  else if (c == '.') {c = 14;}
  else if (c == '/') {c = 15;}  

  else if (c >= '0' && c <= '9') {
    c = (c - '0') + 16;
  }

  else if (c == ':') {c = 26;} 
  else if (c == ';') {c = 27;} 
  else if (c == '<') {c = 28;}
  else if (c == '=') {c = 29;}
  else if (c == '>') {c = 30;}
  else if (c == '?') {c = 31;}
  else if (c == '@') {c = 32;}   

  else if (c >= 'A' && c <= 'Z') {
    c = (c - 'A') + 33;
  }

  else if (c == '[') {c = 59;}
  //else if (c == '\') {c = 60;}
  else if (c == ']') {c = 61;}
  else if (c == '^') {c = 62;}
  else if (c == '_') {c = 63;}
  else if (c == '`') {c = 64;}

  else if (c >= 'a' && c <= 'z') {
    c = (c - 'a') + 65;
  }

  else if (c == '{') {c = 91;}
  else if (c == '|') {c = 92;}
  else if (c == '}') {c = 93;}

/*
  for (char col=0; col< 6; col++) {
    dots = pgm_read_byte_near(&my3font[c][col]);
    //if ((attr && BOLD) && (col > 0)) {
      //dots |= pgm_read_byte_near(&my3font[c][col-1]);
    //}
    //if ((attr && ITALIC) && (col > 0)) {
      //dots |= pgm_read_byte_near(&my3font[c][col-1]);
    //}
    for (char row=0; row <=7; row++) {
      if (dots & (64>>row))   	     // only 7 rows.
        dotmatrix.plot(x+col, y+row, color);
      else 
        dotmatrix.plot(x+col, y+row, BLACK);
      //if ((attr && UNDERLINE) && (row == 7))
        //dotmatrix.plot(x+col, y+row, color);
    }
  }
*/
  /*for (char col=0; col< 6; col++) {
    dots = pgm_read_byte_near(&my3font[c][col]);
    for (char row=0; row <=7; row++) {
      if (dots & (128>>row))
        dotmatrix.plot2(x+col, y+row, color);
      else 
        dotmatrix.plot2(x+col, y+row, BLACK);
    }
  }*/
  /*if (!(y & 7)) {
    for (char col=0; col< 6; col++) {
      dots = pgm_read_byte_near(&my3font[c][col]) << 1;
      byte x1 = x + col;
      word addr = x1 + (x1 & ~15) + 2*(x1 & ~31) + 8*(y & ~7);
      ht1632_update_shadowram(addr, (color & GREEN), dots);
      ht1632_update_shadowram(addr+16, (color & RED), dots);
    }
  }*/


  byte len = sizeof(my3font[c]); 
  int s = 0;

  if (x < -len || x > x_max || y < 0 || y > y_max)
    return;

  if ((x + len) > 63)
    len -= ((x + len) & 63);

  word addr = x;

  if (x < 0) {
    s = -x;
    len -= s;
    addr = 0;
  }

  addr += 8*(y & ~7);

 if (color & MULTICOLOR) color += RED | GREEN;
 if (color & GREEN) {
    memcpy_P(&framebuffer[addr],&my3font[c][s],len);
  } else {
    memset(&framebuffer[addr],0,len);
  }
  if (color & RED) {
    memcpy_P(&framebuffer[addr+128],&my3font[c][s],len);
  } else {
    memset(&framebuffer[addr+128],0,len);
  }
  if (color & MULTICOLOR) {
    byte mask[8] = {0};
    for (byte i = 0; i < len; i++) {
      byte mask = 2*random(127);
      if (random(2)) framebuffer[addr+i] &= ~mask;
      if (random(2)) framebuffer[addr+128+i] &= mask;
    }
  }
  sendframe();
}

/* text only scrolling */

void ht1632c::scrolltextxcolor(int y, const char *text, byte color, int delaytime, int times, byte dir)
{
  int c = 0, x, len = strlen(text) + 1;
  byte showcolor;
  while (times) {
    for ((dir) ? x = - (len * 6) : x = x_max; (dir) ? x <= x_max : x > - (len * 6); (dir) ? x++ : x--)
    {
      for (int i = 0; i < len; i++)
      {
        (color & RANDOMCOLOR) ? showcolor = random(3) + 1 : showcolor = color;
        ((color & BLINK) && (x & 2)) ? showcolor = BLACK : showcolor = showcolor;
        putchar(x + 6 * i,  y, text[i], showcolor);
      }
      c++;
      //delay(delaytime+abs((int)(c-(float)(len*3+(x_max/2)))/2));
      delay(delaytime);
    }
    times--;
  }
}

/* graphic primitives based on Bresenham's algorithms */

void ht1632c::line(int x0, int y0, int x1, int y1, byte color)
{
  int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = dx+dy, e2; /* error value e_xy */

  for(;;) {
    plot(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2*err;
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
  int x = -r, y = 0, err = 2-2*r; /* II. Quadrant */ 
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
  int sx = x0 < x2 ? 1 : -1, sy = y0<y2 ? 1 : -1; /* step direction */
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

byte ht1632c::getpixel (byte x, byte y)
{
  word addr = (x & 63) + 8*(y & ~7);
  byte g = framebuffer[addr];
  byte r = framebuffer[addr+128];
  byte valbit = 1 << (7 - y & 7);
  return (g & valbit) ? GREEN : BLACK | (r & valbit) ? RED : BLACK;
}

/* boundary flood fill with the seed in x, y coordinates */

void ht1632c::fill_r(byte x, byte y, byte color)
{
  if(!getpixel(x, y))
  {
    plot(x, y, color);
    fill_r(++x, y ,color);
    x = x - 1 ;
    fill_r(x, y-1, color);
    fill_r(x, y+1, color);
  }
}

void ht1632c::fill_l(byte x, byte y, byte color)
{
  if(!getpixel(x, y))
  {
    plot(x, y, color);
    fill_l(--x, y, color);
    x = x + 1 ;
    fill_l(x, y-1, color);
    fill_l(x, y+1, color);
  }
}

void ht1632c::fill(byte x, byte y, byte color)
{
  fill_r(x, y, color);
  fill_l(x-1, y, color);
}

