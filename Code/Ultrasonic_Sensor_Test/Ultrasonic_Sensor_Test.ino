#define trigPin1 12
#define echoPin1 13
#define trigPin2 15
#define echoPin2 14


float ultrasonicPulse(int trigPin, int echoPin) {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  int duration = pulseIn(echoPin, HIGH);
  return (duration / 2) / 74.07;
}


void setup() {
  Serial.begin(9600);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  Serial.println("Starting");
}

void loop() {
  Serial.print("Sensor 1: ");
  Serial.print(ultrasonicPulse(trigPin1, echoPin1));
  Serial.println(" in");
  Serial.print("Sensor 2: ");
  Serial.print(ultrasonicPulse(trigPin2, echoPin2));
  Serial.println(" in");
  delay(1000);
}
