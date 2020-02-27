void setup() {
  pinMode(8, INPUT);
  Serial.begin(9600);
}

void loop() {
int reading = digitalRead(8);
Serial.println(reading);
delay(100);
}
