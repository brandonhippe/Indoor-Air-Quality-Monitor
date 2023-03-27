#include "sps30.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Initializing SPS30");
  sps30_setup();
  Serial.println("Initialized SPS30");
}

void loop() {
  // put your main code here, to run repeatedly:
  int measurements[2] = {MCPM2p5, NCPM2p5};
  uint16_t *pm2p5 = sps30_getVal(measurements, 2);
  Serial.print("PM 2.5 Mass Concentration: ");
  Serial.print(pm2p5[0]);
  Serial.println(" ug/m3");
  Serial.print("PM 2.5 Number Concentration: ");
  Serial.print(pm2p5[1]);
  Serial.println(" #/cm3");
  delay(1000);
}
