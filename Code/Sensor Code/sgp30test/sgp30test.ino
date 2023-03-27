#include "sgp30.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Initializing SGP30");
  sgp30_setup();
  Serial.println("Initialized SGP30");
}

void loop() {
  // put your main code here, to run repeatedly: 
  Serial.print("eCO2: ");
  Serial.print(sgp30_getCO2());
  Serial.println(" ppm");
  delay(1000);
}
