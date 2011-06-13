#include <ht1632c.h>

ht1632c dotmatrix = ht1632c(PORTD, 7, 6, 4, 5, GEOM_32x16, 2);

void setup ()
{
  dotmatrix.clear();
}

void loop ()
{
  dotmatrix.line(1,1,10,1,RED);
  dotmatrix.line(1,1,10,10,RED);
  dotmatrix.line(1,1,1,10,RED);
  dotmatrix.circle(20,6,5,RED);
  dotmatrix.ellipse(32,0,63,15,GREEN);
  dotmatrix.rect(40,3,55,12,RED);
  dotmatrix.sendframe();
  while(1);
}
