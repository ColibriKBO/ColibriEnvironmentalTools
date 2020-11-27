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
 W5500   -> Arduino
 MOSI   -> 11
 MISO   -> 12
 SCLK   -> 13
 SS     -> 10
 RST    -> 9
 5V     -> 5V
 GND    -> GND

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
 A0     -> A1

 IR     -> Arduino
 GND    -> GND
 VIN    ->
 SDA    -> A4(SDA)
 SCL    -> A5(SCL)

 Photo  -> Arduino
 5V     -> 5V
 Out    -> A0

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

*///////////////////////////////////////////////////////
#include <Ethernet.h>
#include <Wire.h>
#include "SparkFunBME280.h"

/////////////////////////////
// Define attached sensors //
/////////////////////////////
const bool LIGHTSENSOR  = true;
const bool RAINSENSOR   = true;
const bool BME280IN     = false;
const bool BME280OUT    = false;
const bool CLOUDMONITOR = false;
const bool RELAYS       = false;

// Is the ethernet adapter attached?
const bool W5500        = false;

//////////////////////////////////
// Setup light sensor variables //
//////////////////////////////////
int lightPin = 0;
int lightReading;
const int numReadings = 10;
int total = 0;
int average = 0;
float lux;

/////////////////////////////////
// Setup rain sensor variables //
/////////////////////////////////
int rainPin = 1;
int rainReading;
const int rainMin = 175;
const int rainMax = 1024;
int rainStatus;

////////////////////////////
// Setup BME280 variables //
////////////////////////////
BME280 inSensor;
BME280 outSensor;
int loc; // 0 = "inside" or 1 = "outside"

///////////////////////////
// Setup relay variables //
///////////////////////////
const int r1Pin = 4;
const int r2Pin = 5;
const int r3Pin = 6;
const int r4Pin = 7;

long lastDebounceTime = 0;
long debounceDelay = 10000;

volatile byte r1State = LOW;
String r1s = "Off";
volatile byte r2State = LOW;
String r2s = "Off";
volatile byte r3State = LOW;
String r3s = "Off";
volatile byte r4State = LOW;
String r4s = "Off";

//////////////////////////////////
// Setup cloud monitor variable //
//////////////////////////////////

////////////////////////////////////
// Setup W5500 ethernet variables //
////////////////////////////////////
const byte mac[] = {0xBA, 0xD1, 0xDE, 0xA5, 0x01, 0x00};
IPAddress ip('10.0.20.27');
EthernetServer server(80);
char linebuf[80];
int charcount=0;

void setup() {
  Serial.begin(9600);

  if ( W5500 == true ) {
    // Reset W5500
    digitalWrite(9,LOW);
    delayMicroseconds(500);
    digitalWrite(9,HIGH);
    delayMicroseconds(1000);

     Ethernet.begin(mac, ip);
     server.begin();
     Serial.print("Server is at: ");
     Serial.println(Ethernet.localIP());
  }

  if ( RELAYS == true ) {
    pinMode(r1Pin, OUTPUT);
    digitalWrite(r1Pin, HIGH);
    pinMode(r2Pin, OUTPUT);
    digitalWrite(r2Pin, HIGH);
    pinMode(r3Pin, OUTPUT);
    digitalWrite(r3Pin, HIGH);
    pinMode(r4Pin, OUTPUT);
    digitalWrite(r4Pin, HIGH);
  }

  Wire.begin();
  Wire.setClock(100000);

  if ( BME280IN == true ) {
    inSensor.settings.I2CAddress = 0x76;
    if(inSensor.beginI2C() == false) Serial.println("Sensor A connect failed");
    inSensor.setReferencePressure(101200);
  }

  if ( BME280OUT == true ) {
    outSensor.settings.I2CAddress = 0x77;
    if(outSensor.beginI2C() == false) Serial.println("Sensor A connect failed");
    outSensor.setReferencePressure(101200);
  }

}



void loop() {
  if ( W5500 == true ) {
    EthernetClient client = server.available();
    if (client) {
      Serial.println("New client");
      memset(linebuf,0,sizeof(linebuf));
      charcount = 0;
      boolean currentLineIsBlank = true;

      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          linebuf[charcount] = c;
          if (charcount < sizeof(linebuf)-1) charcount++;
          if (c == '\n' && currentLineIsBlank) {
            dashboard(client);
            break;
          }
          if (c == '\n') {
            if (strstr(linebuf, "GET /relay1off") > 0) {
              digitalWrite(r1Pin, LOW);
              r1s = "Off";
            }
            else if (strstr(linebuf, "GET /relay1on") > 0) {
              digitalWrite(r1Pin, HIGH);
              r1s = "On";

            if (strstr(linebuf, "GET /relay2off") > 0) {
              digitalWrite(r2Pin, HIGH);
              r2s = "Off";
            }
            else if (strstr(linebuf, "GET /relay2on") > 0) {
              digitalWrite(r2Pin, LOW);
              r2s = "On";
            }

            if (strstr(linebuf, "GET /relay3off") > 0) {
              digitalWrite(r3Pin, HIGH);
              r3s = "Off";
            }
            else if (strstr(linebuf, "GET /relay3on") > 0) {
              digitalWrite(r3Pin, LOW);
              r3s = "On";

            if (strstr(linebuf, "GET /relay4off") > 0) {
              digitalWrite(r4Pin, HIGH);
              r4s = "Off";
            }
            else if (strstr(linebuf, "GET /relay4on") > 0) {
              digitalWrite(r4Pin, LOW);
              r4s = "On";
            }

            currentLineIsBlank = true;
            memset(linebuf,0,sizeof(linebuf));
            charcount = 0;
          }
          else if (c != '\r') {
            currentLineIsBlank = false;
        }
      }
      delay(1);
      client.stop();
      Serial.println("Client disconnected...");
    }
        }
      }
    }
  }

  lux = getLux();
  rainStatus = getRain();

  // Wait for a bit before starting loop again
  delay(2000);        // delay in between reads for stability
}

////////////////////////////////////////////////////
// Function to get the ambient light level in lux //
////////////////////////////////////////////////////
float getLux() {
    // Average light sensor readings over 'numReadings'
  for (int i = 0; i < numReadings; i++){
    lightReading = analogRead(lightPin);
    total = total + lightReading;
    //Serial.println(total);
    delay(10);
  }
  average = total / numReadings;
  total = 0;

  // Convert analog light reading to lux
  lux = pow(10,6.6162) * pow(average,-1.9191);

  // Print readings to serial monitor
  Serial.print("Reading: "); Serial.print(average);
  Serial.print(" ==> "); Serial.print(lux); Serial.println(" lux");
  return lux;
}

//////////////////////////////////////////////////////
// Function to return the status of the rain sensor //
//////////////////////////////////////////////////////
int getRain() {
  // Read value of rain sensor and map to an integer between 0 and 4
  rainReading = analogRead(rainPin);
  rainStatus = map(rainReading, rainMin, rainMax, 0, 5);

  // Check rain status and print out results
  switch (rainStatus) {
      case 0:
        Serial.println("RAIN");
        break;
      case 1:
        Serial.println("RAIN");
        break;
      case 2:
        Serial.println("FEW DROPS");
        break;
      case 3:
        Serial.println("MIST");
        break;
      case 4:
        Serial.println("DRY");
        break;
  }
  return rainStatus;
}

////////////////////////////////////////////////////////////////
// Function to return the temperature, humidity, and pressure //
////////////////////////////////////////////////////////////////
float getTHP(int loc) {
    float temp, humi, dewp, pres;
  if ( loc == 0 ) {
    temp = inSensor.readTempC();
    humi = inSensor.readFloatHumidity();
    dewp = inSensor.dewPointC();
    pres = inSensor.readFloatPressure();
  }
  else {
    temp = outSensor.readTempC();
    humi = outSensor.readFloatHumidity();
    dewp = outSensor.dewPointC();
    pres = outSensor.readFloatPressure();
  }
  return temp, humi, dewp, pres;
}

////////////////////////////
// Function to set relays //
////////////////////////////

///////////////////////////////////
// Function to return cloudiness //
///////////////////////////////////

/////////////////////////////////////
// Function to setup web dashboard //
/////////////////////////////////////
void dashboard(EthernetClient &client) {
  client.println("<!DOCTYPE HTML><html><head>");
  client.println("<meta name=\"viewport\" content=\"width=device, initial-scale=1\"></head><body>");
  client.println("<h3>Arduino Web Server - <a href=\"/\">Refresh</a></h3>");

  client.println("<h4>Relay 1 - State: " + r1s + "</h4>");
  if(r1s == "Off") {
    client.println("<a href=\"/relay1on\"><button>ON</button></a>");
  }
  else if(r1s == "On") {
    client.println("<a href=\"/relay1off\"><button>OFF</button></a>");
  }

  client.println("<h4>Relay 2 - State: " + r2s + "</h4>");
  if(r2s == "Off") {
    client.println("<a href=\"/relay2on\"><button>ON</button></a>");
  }
  else if(r2s == "On") {
    client.println("<a href=\"/relay2off\"><button>OFF</button></a>");
  }

  client.println("<h4>Relay 3 - State: " + r3s + "</h4>");
  if(r3s == "Off") {
    client.println("<a href=\"/relay3on\"><button>ON</button></a>");
  }
  else if(r3s == "On") {
    client.println("<a href=\"/relay3off\"><button>OFF</button></a>");
  }

  client.println("<h4>Relay 4 - State: " + r4s + "</h4>");
  if(r4s == "Off") {
    client.println("<a href=\"/relay4on\"><button>ON</button></a>");
  }
  else if(r4s == "On") {
    client.println("<a href=\"/relay4off\"><button>OFF</button></a>");
  }

  client.println("</body></html>");
}
