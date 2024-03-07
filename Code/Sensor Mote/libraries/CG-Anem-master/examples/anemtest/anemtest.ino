#include <cgAnem.h>
#include <NewWire.h>


#define SDA_PIN 10    // Set the SDA pin to 10
#define SCL_PIN 9     // Set the SCL pin to 9
#define CG_SLEEP_PIN 7 	// Set Anemometer sleep pin to 7


uint64_t time_ms;


CG_Anem cganem;


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
  Wire.setClock(cganem.max_clock);
  bool sensed = cganem.begin(CG_SLEEP_PIN, true);
  if (!sensed) {
    Serial.println("ERROR: CG_ANEM NOT CONNECTED!");
    while (1);
  } else {
    Serial.println("CG_ANEM CONNECTED!");
  }

  // Set time_ms and delay to start time
  time_ms = cganem.time_ms;
  while (millis() < time_ms);
}

void loop() {
  // Execute next function
  Serial.println("Executing");
  cganem.startNextFunc(millis());
  if (cganem.measurement_ready) {
    Serial.println(cganem.airflowRate);
  }

  // Sleep until next function
  Serial.println("Sleeping");
  sleep(cganem.time_ms - millis());
}
