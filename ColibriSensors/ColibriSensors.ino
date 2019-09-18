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

inSensor.setSampling(Adafruit_BME280::MODE_FORCED,
  Adafruit_BME280::SAMPLING_X1, // temperature
  Adafruit_BME280::SAMPLING_X1, // pressure
  Adafruit_BME280::SAMPLING_X1, // humidity
  Adafruit_BME280::FILTER_OFF   );

outSensor.setSampling(Adafruit_BME280::MODE_FORCED,
  Adafruit_BME280::SAMPLING_X1, // temperature
  Adafruit_BME280::SAMPLING_X1, // pressure
  Adafruit_BME280::SAMPLING_X1, // humidity
  Adafruit_BME280::FILTER_OFF   );
                      
delayTime = 60000; // in milliseconds

void setup() {
  inSensor.settings.commInnterface=I2C_MODE;
  inSensor.settings.I2CAddress = 0x76;

  outSensor.settings.commInnterface=I2C_MODE;
  outSensor.settings.I2CAddress = 0x77;

  Serial.begin(9600);
  Serial.println("Starting sensors... result of .begin():");
  delay(10);
  Serial.print("Inside sensor found at: 0x");
  Serial.println(inSensor.begin(), HEX);
  Serial.print("Outside sensor found at: 0x");
  Serial.println(outSensor.begin(), HEX);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(delayTime);
}
