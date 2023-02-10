#define trig 12
#define echo1 13
#define echo2 14


#define halfD 0.125


unsigned int ultrasonicPulse(int trigPin, int echoPin) {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH);
}


float calcWindSpeed(unsigned int forward, unsigned int back) {
  float invForward = 1 / forward, invBack = 1 / back;
  return halfD * (invForward - invBack);
}


float calcTemperature(unsigned int forward, unsigned int back) {
  float invForward = 1 / forward, invBack = 1 / back;
  return halfD * (invForward + invBack);
}


void setup() {
  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(echo2, INPUT);
  Serial.println("Starting");
}

void loop() {
  unsigned int forward = ultrasonicPulse(trig, echo1);
  delay(100);
  unsigned int back = ultrasonicPulse(trig, echo2);
  float windspeed = calcWindSpeed(forward, back), temperature = calcTemperature(forward, back);
  char printstring[100];
  sprintf(printstring, "Wind speed: %f m/s\nTemperature: %f degrees\n", windspeed, temperature);
  Serial.print(printstring);
  delay(900);
}
