# Arduino Port Manupulation #

Blinking a led, the standard way:

```
void setup() {
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH);   // set the LED on
  delay(1000);              // wait for a second
  digitalWrite(13, LOW);    // set the LED off
  delay(1000);              // wait for a second
}
```

...the fastest I/O way:

```
struct bits {
  uint8_t b0:1;
  uint8_t b1:1;
  uint8_t b2:1;
  uint8_t b3:1;
  uint8_t b4:1;
  uint8_t b5:1;
  uint8_t b6:1;
  uint8_t b7:1;
} __attribute__((__packed__));

#define SBIT(port,pin) ((*(volatile struct bits*)&port).b##pin)

#define led      SBIT( PORTB, 5 ) // PORTB maps to Arduino digital pins 8 to 13
#define led_ddr  SBIT( DDRB,  5 ) // PORTB maps to Arduino digital pins 8 to 13

void setup() {
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  led_ddr = 1; // OUTPUT
}

void loop() {
  led = 1;                  // set the LED on
  delay(1000);              // wait for a second
  led = 0;                  // set the LED off
  delay(1000);              // wait for a second
}
```


## Helpful links ##

  * [Arduino Reference - Port Manupulation](http://www.arduino.cc/en/Reference/PortManipulation)
  * [GCC Documentation - Specifying Attributes of Variables](http://gcc.gnu.org/onlinedocs/gcc/Variable-Attributes.html)
  * [A Tour of the Arduino Internals: How does Hello World actually work?](http://urbanhonking.com/ideasfordozens/2009/05/18/an_tour_of_the_arduino_interna/)