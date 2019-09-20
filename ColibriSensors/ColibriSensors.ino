/*
Code for the Colibri environmental sensors and relays in each dome.

Date: Sept. 17, 2019
License: GPL

 Sensors List
 ------------
 Temperature
 Humidity
 Pressure
 Rain
 IR

 Connections
 -----------------
 BME280 -> Arduino
 GND    -> GND
 VCC    -> 5V
 SDA    -> A4(SDA)
 SCL    -> A5(SCL)
 SDO    -> GND

 Rain   -> Arduino
 VCC    -> 5V
 GND    -> GND
 D0     -> N.C. (read from D or A)
 A0     -> A0

 IR     -> Arduino
 GND    -> GND
 VIN    ->
 SDA    -> A4(SDA)
 SCL    -> A5(SCL)

 Photo  -> Arduino
 5V     -> 5V
 Out    -> A1
 
 Relay  -> Arduino
 GND    -> GND
 VCC    -> 5V
 IN1    -> D4
 IN2    -> D5
 IN3    -> D6
 IN4    -> D7
 
 (SDO is pin 5 on the sensor package. On our board it
 is connected to the middle pad of the 3 pads to the
 lower left of the package. Cut the trace between the
 two on the left and jump the middle and right pads to
 get 0x77.
 
*/

#include <Wire.h>
#include "SparkFunBME280.h"

BME280 inSensor;
BME280 outSensor;

const int rainMin = 0;
const int rainMax = 1024;

const int rainPin = 0;

const int photoPin = 1;

const int r1Pin = 4;
const int r2Pin = 5;
const int r3Pin = 6;
const int r4Pin = 7;

volatile byte r1State = LOW;
volatile byte r2State = LOW;
volatile byte r3State = LOW;
volatile byte r4State = LOW;

void setup() {
  pinMode(r1Pin, OUTPUT);
  digitalWrite(r1Pin, LOW);
  pinMode(r2Pin, OUTPUT);
  digitalWrite(r2Pin, LOW);
  pinMode(r3Pin, OUTPUT);
  digitalWrite(r3Pin, LOW);
  pinMode(r4Pin, OUTPUT);
  digitalWrite(r4Pin, LOW);
  
  Serial.begin(9600);
  
  Wire.begin();
  Wire.setClock(100000);

  inSensor.setI2CAddress = 0x76;
  if(inSensor.beginI2C() == false) Serial.println("Sensor A connect failed");
  
  outSensor.setI2CAddress = 0x77;
  if(outSensor.beginI2C() == false) Serial.println("Sensor A connect failed");

  inSensor.setReferencePressure(101200);
  outSensor.setReferencePressure(101200);
}

void loop() {
  int rainReading = analogRead(A0);
  int rainStatus = map(rainReading, rainMin, rainMax, 3, 0);

  photocellReading = analogRead(photoPin);
  
  Serial.print(" TempA: ");
  Serial.print(mySensorA.readTempC(), 2);

  Serial.print(" TempB: ");
  Serial.print(mySensorB.readTempC(), 2);
  
  Serial.print("HumidityA: ");
  Serial.print(inSensor.readFloatHumidity(), 0);

  Serial.print(" HumidityB: ");
  Serial.print(outSensor.readFloatHumidity(), 0);

  Serial.print(" DewpointA: ");
  Serial.print(inSensor.dewPointC(), 2);

  Serial.print(" DewpointB: ");
  Serial.print(outSensor.dewPointC(), 2);

  Serial.print(" PressureA: ");
  Serial.print(inSensor.readFloatPressure(), 0);

  Serial.print(" PressureB: ");
  Serial.print(outSensor.readFloatPressure(), 0);

  Serial.print(" Locally Adjusted Altitude: ");
  Serial.print(inSensor.readFloatAltitudeMeters(), 1);

  Serial.print(" Locally Adjusted Altitude: ");
  Serial.print(outSensor.readFloatAltitudeMeters(), 1);

  Serial.print(" Rain Flag: ");
  Serial.print(range);

  Serial.print(" Rain Value: ");
  Serial.print(rainStatus);

  Serial.print(" IR1 Reading: ");

  Serial.print(" IR2 Reading: ");

  Serial.print(" Relay State: ");

  Serial.println();

  delay(5000);
}
