#include <digitalWriteFast.h>
#include <HT1632C.h>

HT1632C ledMatrix = HT1632C();

void setup() {
  Serial.begin(9600);
  ledMatrix.setup();
}

void loop() {
  for (int i=0; i<64; i++) {
    for (int j=0; j<16; j++) {
      ledMatrix.shadowram_plot(i, j, ORANGE);
      ledMatrix.update_display();
    }
  }
  
  for (int i=0; i<64; i++) {
    for (int j=0; j<16; j++) {
      ledMatrix.shadowram_plot(i, j, BLACK);
      ledMatrix.update_display();
    }
  }
}