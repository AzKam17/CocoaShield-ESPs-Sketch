#define LED 2

void setup() {
  Serial.begin(115200);
  pinMode(LED,OUTPUT);
}

void loop() {
  Serial.println("Hello World");
  delay(500);
  digitalWrite(LED,HIGH);
  delay(500);
  digitalWrite(LED,LOW);
}
