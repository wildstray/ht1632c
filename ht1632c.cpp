#include <avr/io.h> 
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <WProgram.h>
#include "pins_arduino.h"
#include "font.h"
#include "font2.h"
#include "font3.h"

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

void ht1632c::pwm(byte value)
{
  sendcmd(HT1632_CS_ALL, HT1632_CMD_PWM | value);
}

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

void ht1632c::scrolltextxcolor(int y, const char *text, byte color, int delaytime, int times = 1, byte dir = LEFT)
{
  int c = 0, x, len = strlen(text) + 1;
  byte showcolor;
  while (times) {
    for ((dir) ? x = -(len * 6) : x = x_max; (dir) ? x <= x_max : x > -(len * 6); (dir) ? x++ : x--)
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

