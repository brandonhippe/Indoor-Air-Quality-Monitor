#include <sgp30.h>
#include <Wire.h>


#define SDA_PIN 10    // set the SDA pin to 10
#define SCL_PIN 9    // set the SCL pin to 9


// Enable serial printing
#define DEBUG


uint64_t time_ms;


SGP30 sgp30;


void setup() {
  for (int i = 0; i < 40; i++) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Starting");

  Wire.setModule(0);
  Wire.begin();
  Wire.setClock(sgp30.max_clock);
  sgp30.begin(millis());

  // Set time_ms and delay to start time
  time_ms = sgp30.time_ms;
  while (millis() < time_ms);
}

void loop() {
  // Execute next function
  Serial.println("Executing");
  sgp30.startNextFunc(millis());
  if (sgp30.measurement_ready) {
    Serial.println(sgp30.co2);
  }

  // Sleep until next function
  Serial.println("Sleeping");
  sleep(sgp30.time_ms - millis());
}
