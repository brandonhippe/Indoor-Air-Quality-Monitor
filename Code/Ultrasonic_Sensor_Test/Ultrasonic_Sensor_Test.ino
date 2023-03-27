#define trig 12
#define echo1 13
#define echo2 14


const float halfD = 0.05;


unsigned int ultrasonicPulse(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH);
}


float calcWindSpeed(unsigned int forward, unsigned int back) {
  float invForward = 1000000.0 / float(forward);
  float invBack = 1000000.0 / float(back);
  return halfD * (invForward - invBack);
}


float calcTemperature(unsigned int forward, unsigned int back) {
  float invForward = 1000000.0 / float(forward);
  float invBack = 1000000.0 / float(back);
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
//  unsigned int back = ultrasonicPulse(trig, echo2);
  Serial.println(forward);
//  Serial.println(back);
//  float windspeed = calcWindSpeed(forward, back), temperature = calcTemperature(forward, back);
//  char windStr[10], tempStr[10];
//  dtostrf(windspeed, 2, 2, windStr);
//  dtostrf(temperature, 2, 2, tempStr); 
//  char printstring[100];
//  sprintf(printstring, "Wind speed: %s m/s\nTemperature: %s degrees", windStr, tempStr);
//  Serial.println(printstring);
}
