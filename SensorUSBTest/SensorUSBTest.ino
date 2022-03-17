/* 
BME280 -> Arduino
GND    -> GND
VCC    -> 5V
SDA    -> A4(SDA)
SCL    -> A5(SCL)
SDO    -> GND

Photo  -> Arduino
5V     -> 5V
Out    -> A1

*/

#include <Wire.h>
#include "SparkFunBME280.h"

BME280 inSensor;
// BME280 outSensor;

const int numReadings = 10;
int reading;
int total = 0;
int average = 0;

int lightPin = 1;
float lux;

void setup() {
  Serial.begin(9600);

  inSensor.settings.I2CAddress = 0x77;
  if(inSensor.beginI2C() == false) Serial.println("Sensor A connect failed");
  inSensor.setReferencePressure(101200);
  
}

void loop() {

  float tempIn = inSensor.readTempC();
  // float tempOut = outSensor.readTempC();
  float humIn = inSensor.readFloatHumidity();
  // float humOut = outSensor.readFloatHumidity();
  float dewIn = inSensor.dewPointC();
  // float dewOut = outSensor.dewPointC();
  
  for (int i = 0; i < numReadings; i++){
    reading = analogRead(lightPin);
    total = total + reading;
    //Serial.println(total);
    delay(100);
  }
  average = total / numReadings;
  total = 0;
  lux = pow(10,6.6162) * pow(average,-1.9191);
  
  Serial.print("Reading: "); Serial.print(average);
  Serial.print(" ==> "); Serial.print(lux); Serial.println(" lux");
  Serial.print("Temp Inside: "); Serial.println(tempIn);
  Serial.print("Hum Inside: "); Serial.println(humIn);
  Serial.print("Dewpoint: "); Serial.println(dewIn);
  
  delay(5000);        // delay in between reads for stability
}
