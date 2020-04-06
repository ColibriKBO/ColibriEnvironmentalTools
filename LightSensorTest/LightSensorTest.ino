const int numReadings = 10;
int reading;
int total = 0;
int average = 0;

int lightPin = 0;
float lux;

void setup() {
  Serial.begin(9600);
}

void loop() {
  for (int i = 0; i < numReadings; i++){
    reading = analogRead(lightPin);
    total = total + reading;
    Serial.println(total);
    delay(100);
  }
  average = total / numReadings;
  total = 0;
  lux = pow(10,6.6162) * pow(average,-1.9191);
  Serial.print("Reading: "); Serial.print(average);
  Serial.print(" ==> "); Serial.print(lux); Serial.println(" lux");
  delay(1000);        // delay in between reads for stability
}
