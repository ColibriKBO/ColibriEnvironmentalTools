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

#include <Ethernet2.h>
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

const byte mac[] = {0xBA, 0xD1, 0xDE, 0xA5, 0x01, 0x00};
IPAddress ip('10.0.20.27');
//IPAddress ip(192,168,1,10);
EthernetServer server(80);

char linebuf[80];
int charcount=0;

byte setRelay(char rpin, char relayState, float tempDelta) {
  if (tempDelta < 15 && relayState == LOW) {
    digitalWrite(r4Pin, LOW);
    relayState = HIGH;
    lastDebounceTime = millis();
    Serial.print(" Relay State: ON");
  }
  else if (tempDelta > 15 && relayState == HIGH) {
    digitalWrite(r4Pin, HIGH);
    relayState = LOW;
    Serial.print(" Relay State: OFF");   
  }
  else {
    Serial.print(" Relay State: OFF");
  }
  return relayState;
  }

void setup() {
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  delayMicroseconds(500);
  digitalWrite(9, HIGH);
  delayMicroseconds(1000);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server is at: ");
  Serial.println(Ethernet.localIP());
  
  pinMode(r1Pin, OUTPUT);
  digitalWrite(r1Pin, HIGH);
  pinMode(r2Pin, OUTPUT);
  digitalWrite(r2Pin, HIGH);
  pinMode(r3Pin, OUTPUT);
  digitalWrite(r3Pin, HIGH);
  pinMode(r4Pin, OUTPUT);
  digitalWrite(r4Pin, HIGH);
  
  Serial.begin(9600);
  
  Wire.begin();
  Wire.setClock(100000);

  inSensor.settings.I2CAddress = 0x76;
  if(inSensor.beginI2C() == false) Serial.println("Sensor A connect failed");
  
  outSensor.settings.I2CAddress = 0x77;
  if(outSensor.beginI2C() == false) Serial.println("Sensor A connect failed");

  inSensor.setReferencePressure(101200);
  outSensor.setReferencePressure(101200);
}

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


void loop() {
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


  
//  delay(5000);
//  float tempIn = inSensor.readTempC();
//  float tempOut = outSensor.readTempC();
//  float humIn = inSensor.readFloatHumidity();
//  float humOut = outSensor.readFloatHumidity();
//  float dewIn = inSensor.dewPointC();
//  float dewOut = outSensor.dewPointC();
//  float tempDelta = tempIn - dewIn;
//  
//  int rainReading = analogRead(A0);
//  int rainStatus = map(rainReading, rainMin, rainMax, 3, 0);
//
//  int photocellReading = analogRead(photoPin);
//  
//  Serial.print(" Temp In: ");
//  Serial.print(tempIn, 1);



//  Serial.print(" Temp Out: ");
//  Serial.print(tempOut, 1);


  
//  Serial.print(" Humidity In: ");
//  Serial.print(humIn, 1);



//  Serial.print(" HumidityOut: ");
//  Serial.print(humOut, 1);


//  Serial.print(" Dewpoint In: ");
//  Serial.print(dewIn, 1);


//  Serial.print(" Dewpoint Out: ");
//  Serial.print(dewOut, 1);

//  Serial.print(" Pressure In: ");
//  Serial.print(inSensor.readFloatPressure(), 0);

//  Serial.print(" Pressure Out: ");
//  Serial.print(outSensor.readFloatPressure(), 0);

//  Serial.print(" Locally Adjusted Altitude: ");
//  Serial.print(inSensor.readFloatAltitudeMeters(), 1);

//  Serial.print(" Locally Adjusted Altitude: ");
//  Serial.print(outSensor.readFloatAltitudeMeters(), 1);

//  Serial.print(" Rain Flag: ");
//  Serial.print(range);
//
//  Serial.print(" Rain Value: ");
//  Serial.print(rainStatus);
//
//  Serial.print(" IR1 Reading: ");
//
//  Serial.print(" IR2 Reading: ");

//  Serial.print(" Relay State: ");

//  if (tempIn - dewIn < 2 && r4State == HIGH) {
//    digitalWrite(r4Pin, LOW);
//    r1State = LOW;
//    Serial.print(" Relay State: ON");
//  }
//  else if (tempIn - dewIn > 2 && r4State == LOW) {
//    digitalWrite(r4Pin, HIGH);
//    r1State = HIGH;
//    Serial.print(" Relay State: OFF");   
//  }
//  else {
//    Serial.print(" Relay State: OFF");
//  }


//  if ((millis() - lastDebounceTime) > debounceDelay) {
//    r4State = setRelay(r4Pin, r4State, tempDelta);
//  }
//  else if (r4State == HIGH) {
//    Serial.print(" Relay State: ON");
//  }
//  else Serial.print(" Relay State: OFF");
//
//  Serial.println();

  
//  delay(1000);
//  digitalWrite(r1Pin, LOW);
//  delay(2000);
//  digitalWrite(r1Pin, HIGH);
//  delay(2000);
//  digitalWrite(r2Pin, LOW);
//  delay(500);
//  digitalWrite(r2Pin, HIGH);
//  delay(500);
//  digitalWrite(r3Pin, LOW);
//  delay(500);
//  digitalWrite(r3Pin, HIGH);
//  delay(500);
//  digitalWrite(r4Pin, LOW);
//  delay(500);
//  digitalWrite(r4Pin, HIGH);
//  delay(500);

  
