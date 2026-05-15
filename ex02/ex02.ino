const int ledPin = 2;
unsigned long lastMillis = 0;
const long interval = 1000;

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastMillis >= interval) {
    lastMillis = currentMillis;
    
    int state = digitalRead(ledPin);
    digitalWrite(ledPin, !state);
  }
}