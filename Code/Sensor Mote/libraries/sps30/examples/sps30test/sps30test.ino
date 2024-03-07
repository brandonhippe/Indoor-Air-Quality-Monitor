#include <sps30.h>
#include <NewWire.h>


#define SDA_PIN 10    // Set the SDA pin to 10
#define SCL_PIN 9     // Set the SCL pin to 9


#define SPS_FP true


uint64_t time_ms;


SPS30 sps30;


void setup() {
  for (int i = 0; i < 40; i++) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

  pinMode(SCL_PIN, INPUT_PULLUP);
  pinMode(SDA_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Starting");

  Wire.setModule(0);
  Wire.begin();
  Wire.setClock(sps30.max_clock);
  sps30.begin(MCPM2p5, SPS_FP, millis(), true);

  // Set time_ms and delay to start time
  time_ms = sps30.time_ms;
  while (millis() < time_ms);
}

void loop() {
  // Execute next function
  Serial.println("Executing");
  sps30.startNextFunc(millis());
  if (sps30.measurement_ready) {
    if (sps30.fp) {
      Serial.println(sps30.pm2p5_float);
    } else {
      Serial.println(sps30.pm2p5_int);
    }
  }

  // Sleep until next function
  Serial.println("Sleeping");
  sleep(sps30.time_ms - millis());
}
