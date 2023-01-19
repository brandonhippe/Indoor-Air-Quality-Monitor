#define trigPin 12
#define echoPin 13


void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  digitalWrite(trigPin, HIGH);  
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  int duration = pulseIn(echoPin, HIGH);
  float distance = (duration / 2) / 74.07;
  Serial.print(distance);
  Serial.println(" in");
  delay(10);
}
